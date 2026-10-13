// SPDX-License-Identifier: Apache-2.0
#include "bridge.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

typedef enum { ACTION_TAP, ACTION_DRAG, ACTION_KEY, ACTION_TYPE } action_kind;
typedef struct {
    uint32_t id, epoch;
    action_kind kind;
    uint16_t x, y, end_x, end_y, duration;
    uint8_t key, modifiers, text[120], text_length;
} action;
static QueueHandle_t actions;
static SemaphoreHandle_t socket_lock;
static atomic_uint epoch;
static atomic_bool busy;
static atomic_uint heartbeat_ms;
static int client = -1;

static uint32_t now_ms(void) { return (uint32_t)(esp_timer_get_time() / 1000); }
static bool alive(uint32_t owner) {
    return owner == atomic_load(&epoch) && bridge_usb_online() && !board_stop_pressed()
        && (uint32_t)(now_ms() - atomic_load(&heartbeat_ms)) < 1500;
}
static bool pause_action(uint32_t owner, unsigned milliseconds) {
    uint32_t start = now_ms();
    do {
        if (!alive(owner)) return false;
        vTaskDelay(pdMS_TO_TICKS(5));
    } while ((uint32_t)(now_ms() - start) < milliseconds);
    return alive(owner);
}
static bool write_line_locked(const char *line) {
    if (client < 0) return false;
    size_t length = strlen(line), sent = 0;
    while (sent < length) {
        int result = send(client, line + sent, length - sent, 0);
        if (result <= 0) return false;
        sent += (size_t)result;
    }
    return true;
}
static void write_line(const char *line, uint32_t owner) {
    xSemaphoreTake(socket_lock, portMAX_DELAY);
    if (owner == atomic_load(&epoch) && !write_line_locked(line)) {
        if (client >= 0) shutdown(client, SHUT_RDWR);
        atomic_fetch_add(&epoch, 1);
    }
    xSemaphoreGive(socket_lock);
}

// US keyboard layout; Unicode and IME composition are outside this prototype.
static bool ascii_key(uint8_t c, uint8_t *key, uint8_t *modifiers) {
    *modifiers = 0;
    if (c >= 'a' && c <= 'z') { *key = 4 + c - 'a'; return true; }
    if (c >= 'A' && c <= 'Z') { *key = 4 + c - 'A'; *modifiers = 2; return true; }
    if (c >= '1' && c <= '9') { *key = 30 + c - '1'; return true; }
    if (c == '0') { *key = 39; return true; }
    if (c == '\n') { *key = 40; return true; }
    if (c == '\t') { *key = 43; return true; }
    if (c == ' ') { *key = 44; return true; }
    const char plain[] = "-=[]\\;'`,./";
    const char shifted[] = "_+{}|:\"~<>?";
    const uint8_t codes[] = {45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56};
    for (unsigned i = 0; i < sizeof(codes); ++i) {
        if (c == plain[i] || c == shifted[i]) {
            *key = codes[i]; *modifiers = c == shifted[i] ? 2 : 0; return true;
        }
    }
    const char symbols[] = "!@#$%^&*()";
    for (unsigned i = 0; i < 10; ++i) {
        if (c == symbols[i]) { *key = i == 9 ? 39 : 30 + i; *modifiers = 2; return true; }
    }
    return false;
}
static bool stroke(uint32_t owner, uint8_t modifiers, uint8_t key) {
    return alive(owner) && bridge_key(modifiers, key) && pause_action(owner, 12)
        && bridge_key(0, 0) && pause_action(owner, 12);
}
static bool execute(const action *a) {
    if (!alive(a->epoch)) return false;
    if (a->kind == ACTION_KEY) return stroke(a->epoch, a->modifiers, a->key);
    if (a->kind == ACTION_TYPE) {
        for (unsigned i = 0; i < a->text_length; ++i) {
            uint8_t key, modifiers;
            if (!ascii_key(a->text[i], &key, &modifiers) || !stroke(a->epoch, modifiers, key)) return false;
        }
        return true;
    }
    if (!bridge_mouse(0, a->x, a->y) || !pause_action(a->epoch, 30)
        || !bridge_mouse(1, a->x, a->y) || !pause_action(a->epoch, 40)) return false;
    if (a->kind == ACTION_DRAG) {
        unsigned steps = a->duration / 20;
        for (unsigned i = 1; i <= steps; ++i) {
            int32_t x = a->x + ((int32_t)a->end_x - a->x) * (int32_t)i / (int32_t)steps;
            int32_t y = a->y + ((int32_t)a->end_y - a->y) * (int32_t)i / (int32_t)steps;
            if (!alive(a->epoch) || !bridge_mouse(1, x, y) || !pause_action(a->epoch, 20)) return false;
        }
    }
    return alive(a->epoch) && bridge_mouse(0, a->kind == ACTION_DRAG ? a->end_x : a->x,
        a->kind == ACTION_DRAG ? a->end_y : a->y);
}
static void action_worker(void *arg) {
    (void)arg;
    uint32_t released_epoch = UINT32_MAX;
    for (;;) {
        action next;
        if (xQueueReceive(actions, &next, pdMS_TO_TICKS(10)) == pdTRUE) {
            bool result = execute(&next);
            bridge_release();
            released_epoch = atomic_load(&epoch);
            char reply[32];
            snprintf(reply, sizeof(reply), "%lu %s\n", (unsigned long)next.id, result ? "DONE" : "ERROR");
            atomic_store(&busy, false);
            write_line(reply, next.epoch);
        } else if (released_epoch != atomic_load(&epoch) && bridge_usb_online()) {
            bridge_release(); released_epoch = atomic_load(&epoch);
        }
    }
}
static bool number(const char *token, uint32_t maximum, uint32_t *value) {
    if (!token || !*token || strlen(token) > 10) return false;
    for (const char *p = token; *p; ++p) if (*p < '0' || *p > '9') return false;
    errno = 0; char *end;
    unsigned long parsed = strtoul(token, &end, 10);
    if (errno || *end || parsed > maximum) return false;
    *value = (uint32_t)parsed; return true;
}
static bool parse_action(char *line, uint32_t owner, action *out) {
    char *save, *tokens[8]; size_t count = 0;
    for (char *part = strtok_r(line, " ", &save); part; part = strtok_r(NULL, " ", &save)) {
        if (count == 8) return false;
        tokens[count++] = part;
    }
    memset(out, 0, sizeof(*out)); out->epoch = owner;
    if (count < 3 || !number(tokens[0], UINT32_MAX, &out->id) || out->id == 0) return false;
    if (strcmp(tokens[1], "TYPE") == 0 && count == 3) {
        size_t length = strlen(tokens[2]);
        if (length == 0 || length > 240 || length % 2) return false;
        for (size_t i = 0; i < length / 2; ++i) {
            char hex[3] = {tokens[2][2*i], tokens[2][2*i+1], 0};
            for (unsigned j = 0; j < 2; ++j) if (!((hex[j] >= '0' && hex[j] <= '9') || (hex[j] >= 'a' && hex[j] <= 'f'))) return false;
            uint8_t key, mods, byte = (uint8_t)strtoul(hex, NULL, 16);
            if (!ascii_key(byte, &key, &mods)) return false;
            out->text[i] = byte;
        }
        out->text_length = length / 2; out->kind = ACTION_TYPE; return true;
    }
    uint32_t values[5];
    for (size_t i = 2; i < count; ++i) if (i - 2 >= 5 || !number(tokens[i], 65535, &values[i-2])) return false;
    if (strcmp(tokens[1], "KEY") == 0 && count == 4 && values[0] >= 4 && values[0] <= 231 && values[1] <= 255) {
        out->kind = ACTION_KEY; out->key = values[0]; out->modifiers = values[1]; return true;
    }
    if (strcmp(tokens[1], "TAP") == 0 && count == 4) { out->kind = ACTION_TAP; }
    else if (strcmp(tokens[1], "DRAG") == 0 && count == 7 && values[4] >= 100 && values[4] <= 2000) {
        out->kind = ACTION_DRAG; out->end_x = values[2]; out->end_y = values[3]; out->duration = values[4];
    } else return false;
    out->x = values[0]; out->y = values[1]; return true;
}
static bool handle_line(char *line, uint32_t owner, uint32_t *last_id) {
    if (strcmp(line, "PING") == 0) {
        atomic_store(&heartbeat_ms, now_ms()); write_line("ANAN1 PONG\n", owner); return true;
    }
    if (strcmp(line, "STOP") == 0) { atomic_fetch_add(&epoch, 1); return false; }
    action next;
    if (!parse_action(line, owner, &next) || next.id <= *last_id) return false;
    *last_id = next.id;
    bool expected = false;
    if (!atomic_compare_exchange_strong(&busy, &expected, true)) {
        char reply[32]; snprintf(reply, sizeof(reply), "%lu ERROR\n", (unsigned long)next.id);
        write_line(reply, owner); return true;
    }
    atomic_store(&heartbeat_ms, now_ms());
    if (xQueueSend(actions, &next, 0) != pdTRUE) { atomic_store(&busy, false); return false; }
    return true;
}
static void serve(int peer) {
    struct timeval timeout = {.tv_sec = 0, .tv_usec = 100000};
    setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    struct timeval send_timeout = {.tv_sec = 1};
    setsockopt(peer, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout));
    xSemaphoreTake(socket_lock, portMAX_DELAY);
    client = peer;
    uint32_t owner = atomic_fetch_add(&epoch, 1) + 1;
    atomic_store(&heartbeat_ms, now_ms());
    xSemaphoreGive(socket_lock);
    while (atomic_load(&busy) && alive(owner)) vTaskDelay(pdMS_TO_TICKS(5));
    write_line("ANAN1 READY\n", owner);
    char line[300]; size_t used = 0; uint32_t last_id = 0;
    bool running = true;
    while (running && alive(owner)) {
        char bytes[128]; int count = recv(peer, bytes, sizeof(bytes), 0);
        if (count == 0) break;
        if (count < 0) { if (errno == EAGAIN || errno == EWOULDBLOCK) continue; break; }
        for (int i = 0; i < count; ++i) {
            if (bytes[i] == '\n') {
                line[used] = 0;
                running = handle_line(line, owner, &last_id); used = 0;
                if (!running) break;
            } else if ((unsigned char)bytes[i] < 32 || used == sizeof(line)-1) { running = false; break; }
            else line[used++] = bytes[i];
        }
    }
    xSemaphoreTake(socket_lock, portMAX_DELAY);
    client = -1; atomic_fetch_add(&epoch, 1);
    shutdown(peer, SHUT_RDWR); close(peer);
    xSemaphoreGive(socket_lock);
}
void app_main(void) {
    board_init();
    socket_lock = xSemaphoreCreateMutex(); actions = xQueueCreate(1, sizeof(action));
    configASSERT(socket_lock && actions);
    bridge_usb_init();
    configASSERT(xTaskCreate(action_worker, "hid-actions", 4096, NULL, 5, NULL) == pdPASS);
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    configASSERT(listener >= 0);
    int reuse = 1; setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in address = {.sin_family = AF_INET, .sin_port = htons(28766)};
    address.sin_addr.s_addr = inet_addr("192.168.7.1");
    configASSERT(bind(listener, (struct sockaddr *)&address, sizeof(address)) == 0);
    configASSERT(listen(listener, 1) == 0);
    ESP_LOGI("stopwatch", "ANAN1 ready; fixed USB network, no wireless, both buttons stop input");
    for (;;) {
        int peer = accept(listener, NULL, NULL);
        if (peer >= 0) serve(peer);
        else vTaskDelay(pdMS_TO_TICKS(100));
    }
}

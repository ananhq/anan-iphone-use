// SPDX-License-Identifier: Apache-2.0
// MCP actions retain the original HID execution sequence.
#include "actions.h"
#include "bridge.h"
#include "eyes.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdatomic.h>
#include <limits.h>

typedef struct {
    action a;
    action_guard guard;
    action_done done;
    void *context;
} queued_action;
static QueueHandle_t queue;
static atomic_bool busy;
static atomic_uint release_serial;
static action_guard current_guard;
static void *current_context;
static uint32_t now_ms(void) { return (uint32_t)(esp_timer_get_time() / 1000); }
static bool alive(uint32_t owner) { return current_guard && current_guard(owner, current_context); }
static bool pause_action(uint32_t owner, unsigned milliseconds) {
    uint32_t start = now_ms();
    do {
        if (!alive(owner))
            return false;
        vTaskDelay(pdMS_TO_TICKS(5));
    } while ((uint32_t)(now_ms() - start) < milliseconds);
    return alive(owner);
}
// US keyboard layout; Unicode and IME composition are outside this prototype.
bool ascii_key(uint8_t c, uint8_t *key, uint8_t *modifiers) {
    *modifiers = 0;
    if (c >= 'a' && c <= 'z') {
        *key = 4 + c - 'a';
        return true;
    }
    if (c >= 'A' && c <= 'Z') {
        *key = 4 + c - 'A';
        *modifiers = 2;
        return true;
    }
    if (c >= '1' && c <= '9') {
        *key = 30 + c - '1';
        return true;
    }
    if (c == '0') {
        *key = 39;
        return true;
    }
    if (c == '\n') {
        *key = 40;
        return true;
    }
    if (c == '\t') {
        *key = 43;
        return true;
    }
    if (c == ' ') {
        *key = 44;
        return true;
    }
    const char plain[] = "-=[]\\;'`,./";
    const char shifted[] = "_+{}|:\"~<>?";
    const uint8_t codes[] = {45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56};
    for (unsigned i = 0; i < sizeof(codes); ++i) {
        if (c == plain[i] || c == shifted[i]) {
            *key = codes[i];
            *modifiers = c == shifted[i] ? 2 : 0;
            return true;
        }
    }
    const char symbols[] = "!@#$%^&*()";
    for (unsigned i = 0; i < 10; ++i) {
        if (c == symbols[i]) {
            *key = i == 9 ? 39 : 30 + i;
            *modifiers = 2;
            return true;
        }
    }
    return false;
}
static bool stroke(uint32_t owner, uint8_t modifiers, uint8_t key) {
    return alive(owner) && bridge_key(modifiers, key) && pause_action(owner, 12) &&
           bridge_key(0, 0) && pause_action(owner, 12);
}
static bool tween(uint32_t owner, action_point from, action_point to, unsigned duration) {
    unsigned steps = (duration + 19) / 20;
    for (unsigned i = 1; i <= steps; ++i) {
        uint16_t x = from.x + ((int32_t)to.x - from.x) * (int32_t)i / (int32_t)steps;
        uint16_t y = from.y + ((int32_t)to.y - from.y) * (int32_t)i / (int32_t)steps;
        unsigned wait = duration * i / steps - duration * (i - 1) / steps;
        if (!alive(owner) || !bridge_mouse(1,x,y) || !pause_action(owner,wait)) return false;
    }
    return true;
}
static bool execute_hid(action *a) {
    if (a->kind == ACTION_KEYBOARD) {
        for (unsigned i = 0; i < a->repeat; ++i) {
            if (!alive(a->epoch) || !bridge_key(a->modifiers,a->key) ||
                !pause_action(a->epoch,a->press_ms) || !alive(a->epoch) || !bridge_key(0,0)) return false;
            ++a->completed_units;
            if (i + 1 < a->repeat && !pause_action(a->epoch,a->gap_ms)) return false;
        }
        return alive(a->epoch);
    }
    if (a->kind == ACTION_CONSUMER) {
        if (!alive(a->epoch) || !bridge_consumer(a->consumer) ||
            !pause_action(a->epoch,a->press_ms) || !alive(a->epoch) || !bridge_consumer(0)) return false;
        a->completed_units = 1;
        return alive(a->epoch);
    }
    if (!alive(a->epoch) || !bridge_mouse(0,a->x,a->y) || !pause_action(a->epoch,30)) return false;
    if (a->kind == ACTION_SCROLL) {
        for (unsigned i = 0; i < a->repeat; ++i) {
            if (!alive(a->epoch) || !bridge_scroll(a->x,a->y,a->vertical,a->horizontal)) return false;
            ++a->completed_units;
            if (i + 1 < a->repeat && !pause_action(a->epoch,a->gap_ms)) return false;
        }
        return alive(a->epoch);
    }
    if (a->kind == ACTION_DOUBLE_TAP) {
        for (unsigned i = 0; i < 2; ++i) {
            if (!alive(a->epoch) || !bridge_mouse(1,a->x,a->y) ||
                !pause_action(a->epoch,40) || !alive(a->epoch) || !bridge_mouse(0,a->x,a->y)) return false;
            ++a->completed_units;
            if (!i && !pause_action(a->epoch,a->gap_ms)) return false;
        }
        return alive(a->epoch);
    }
    if (!alive(a->epoch) || !bridge_mouse(1,a->x,a->y)) return false;
    if (a->kind == ACTION_LONG_PRESS) {
        if (!pause_action(a->epoch,a->duration) || !alive(a->epoch) ||
            !bridge_mouse(0,a->x,a->y)) return false;
        a->completed_units = 1;
        return alive(a->epoch);
    }
    if (a->hold_ms && !pause_action(a->epoch,a->hold_ms)) return false;
    if (a->kind == ACTION_DRAG_HOLD) {
        if (!tween(a->epoch,(action_point){a->x,a->y},(action_point){a->end_x,a->end_y},a->duration) ||
            !alive(a->epoch) || !bridge_mouse(0,a->end_x,a->end_y)) return false;
        a->completed_units = 1;
        return alive(a->epoch);
    }
    if (a->kind == ACTION_PATH) {
        unsigned segments = a->point_count - 1;
        for (unsigned i = 0; i < segments; ++i) {
            unsigned duration = a->duration * (i + 1) / segments - a->duration * i / segments;
            if (!tween(a->epoch,a->points[i],a->points[i + 1],duration)) return false;
            ++a->completed_units;
        }
        action_point last = a->points[a->point_count - 1];
        return alive(a->epoch) && bridge_mouse(0,last.x,last.y) && alive(a->epoch);
    }
    return false;
}
static bool execute(action *a) {
    if (!alive(a->epoch))
        return false;
    if (a->kind >= ACTION_KEYBOARD)
        return execute_hid(a);
    if (a->kind == ACTION_KEY)
        return stroke(a->epoch, a->modifiers, a->key);
    if (a->kind == ACTION_TYPE) {
        for (unsigned i = 0; i < a->text_length; ++i) {
            uint8_t key, modifiers;
            if (!ascii_key(a->text[i], &key, &modifiers) || !stroke(a->epoch, modifiers, key))
                return false;
        }
        return true;
    }
    if (!bridge_mouse(0, a->x, a->y) || !pause_action(a->epoch, 30) ||
        !bridge_mouse(1, a->x, a->y) || !pause_action(a->epoch, 40))
        return false;
    if (a->kind == ACTION_DRAG) {
        unsigned steps = a->duration / 20;
        for (unsigned i = 1; i <= steps; ++i) {
            int32_t x = a->x + ((int32_t)a->end_x - a->x) * (int32_t)i / (int32_t)steps;
            int32_t y = a->y + ((int32_t)a->end_y - a->y) * (int32_t)i / (int32_t)steps;
            if (!alive(a->epoch) || !bridge_mouse(1, x, y) || !pause_action(a->epoch, 20))
                return false;
        }
    }
    return alive(a->epoch) && bridge_mouse(0, a->kind == ACTION_DRAG ? a->end_x : a->x,
                                           a->kind == ACTION_DRAG ? a->end_y : a->y);
}
static void action_worker(void *arg) {
    (void)arg;
    uint32_t released = UINT32_MAX;
    for (;;) {
        queued_action next;
        if (xQueueReceive(queue, &next, pdMS_TO_TICKS(10)) == pdTRUE) {
            current_guard = next.guard;
            current_context = next.context;
            eyes_begin(&next.a,now_ms());
            bool result = execute(&next.a);
            bridge_release();
            eyes_end(result,now_ms());
            released = atomic_load(&release_serial);
            atomic_store(&busy, false);
            next.done(&next.a, result, next.context);
            current_context = NULL;
            current_guard = NULL;
        } else if (released != atomic_load(&release_serial) && bridge_input_online()) {
            bridge_release();
            released = atomic_load(&release_serial);
        }
    }
}
void actions_init(void) {
    queue = xQueueCreate(1, sizeof(queued_action));
    configASSERT(queue);
    configASSERT(xTaskCreate(action_worker, "hid-actions", 4096, NULL, 5, NULL) == pdPASS);
}
bool actions_busy(void) { return atomic_load(&busy); }
bool actions_reserve(void) {
    bool expected = false;
    return atomic_compare_exchange_strong(&busy, &expected, true);
}
void actions_unreserve(void) { atomic_store(&busy, false); }
bool actions_enqueue(const action *a, action_guard guard, action_done done, void *context) {
    queued_action next = {*a, guard, done, context};
    if (xQueueSend(queue, &next, 0) == pdTRUE)
        return true;
    atomic_store(&busy, false);
    return false;
}
void actions_request_release(void) { atomic_fetch_add(&release_serial, 1); }

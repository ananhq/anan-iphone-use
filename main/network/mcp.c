// SPDX-License-Identifier: Apache-2.0
#include "mcp.h"
#include "mcp_tools.h"
#include "actions.h"
#include "hid_input.h"
#include "bridge.h"
#include "device.h"
#include "display.h"
#include "touch.h"
#include "identity.h"
#include "cJSON.h"
#include "esp_http_server.h"
#include "wireless.h"
#include "network_authority.h"
#include "discovery.h"
#include "input_backends.h"
#include <errno.h>
#include "esp_https_server.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_app_desc.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"
#include "reset_trace.h"
#include "mbedtls/base64.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/sha256.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include <math.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BODY_LIMIT 4096
#define SESSION_LIMIT 4
#define INPUT_IDS_LIMIT 64
#define SESSION_TTL 600000u
#define SETUP_TTL 600000u
#define VERSION "2025-11-25"
typedef struct {
    char id[33];
    bool initialized, wifi;
    uint32_t network_generation;
    uint32_t touched;
    unsigned input_ids_count;
    unsigned char input_ids[INPUT_IDS_LIMIT][32];
} session;
typedef struct {
    httpd_req_t *req;
    cJSON *id;
    char session_id[33];
    uint32_t generation, device_generation, deadline;
    atomic_bool cancelled;
    bool success, wifi;
    uint32_t network_generation;
    int socket;
    action_kind input_kind;
    uint16_t completed_units, total_units;
    SemaphoreHandle_t done;
} input_request;
typedef struct {
    bool reset;
    time_t now;
    uint32_t setup_generation;
} identity_job;
static SemaphoreHandle_t lock;
static QueueHandle_t identity_jobs;
static httpd_handle_t http_server, https_server;
static atomic_bool http_running, https_running, identity_busy;
static atomic_int http_start_error;
static atomic_uint input_generation;
static const bool secure_route = true;
static session sessions[SESSION_LIMIT];
static input_request *active;
static char owner[33];
static uint32_t owner_generation;
static bool owner_wifi;
static uint32_t setup_until, setup_generation;
static char setup_nonce[33], setup_code[7];
static time_t setup_time;
static enum { PAIR_NONE, PAIR_PENDING, PAIR_WORKING, PAIR_READY, PAIR_FAILED } pair_state;
extern const unsigned char setup_html_start[] asm("_binary_setup_html_start");
extern const unsigned char setup_html_end[] asm("_binary_setup_html_end");
static uint32_t now_ms(void) { return esp_timer_get_time() / 1000; }
static bool within(uint32_t deadline) { return deadline && (int32_t)(deadline - now_ms()) > 0; }
static cJSON *field(const cJSON *o, const char *key) {
    return cJSON_GetObjectItemCaseSensitive(o, key);
}
static bool string_is(const cJSON *v, const char *s) {
    return cJSON_IsString(v) && !strcmp(v->valuestring, s);
}
static bool equal_secret(const char *a, const char *b, size_t length) {
    unsigned char different = 0;
    for (size_t i = 0; i < length; ++i)
        different |= (unsigned char)a[i] ^ (unsigned char)b[i];
    return different == 0;
}
static void response_headers(httpd_req_t *r) {
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
    httpd_resp_set_hdr(r, "Pragma", "no-cache");
    httpd_resp_set_hdr(r, "X-Content-Type-Options", "nosniff");
    httpd_resp_set_hdr(r, "Referrer-Policy", "no-referrer");
}
static esp_err_t send_json(httpd_req_t *r, const char *status, cJSON *body) {
    char *text = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);
    if (!text)
        return ESP_ERR_NO_MEM;
    response_headers(r);
    httpd_resp_set_status(r, status);
    httpd_resp_set_type(r, "application/json");
    esp_err_t result = httpd_resp_send(r, text, HTTPD_RESP_USE_STRLEN);
    mbedtls_platform_zeroize(text, strlen(text));
    cJSON_free(text);
    return result;
}
static esp_err_t http_error(httpd_req_t *r, const char *status, const char *message) {
    cJSON *body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "error", message);
    return send_json(r, status, body);
}
static esp_err_t empty_response(httpd_req_t *r, const char *status) {
    response_headers(r);
    httpd_resp_set_status(r, status);
    return httpd_resp_send(r, NULL, 0);
}
static cJSON *envelope(const cJSON *id) {
    cJSON *reply = cJSON_CreateObject();
    cJSON_AddStringToObject(reply, "jsonrpc", "2.0");
    cJSON_AddItemToObject(reply, "id", id ? cJSON_Duplicate(id, true) : cJSON_CreateNull());
    return reply;
}
static esp_err_t rpc_error(httpd_req_t *r, const cJSON *id, int code, const char *message) {
    cJSON *reply = envelope(id), *error = cJSON_AddObjectToObject(reply, "error");
    cJSON_AddNumberToObject(error, "code", code);
    cJSON_AddStringToObject(error, "message", message);
    return send_json(r, "200 OK", reply);
}
static esp_err_t rpc_result(httpd_req_t *r, const cJSON *id, cJSON *result) {
    cJSON *reply = envelope(id);
    cJSON_AddItemToObject(reply, "result", result);
    return send_json(r, "200 OK", reply);
}
static esp_err_t rpc_tools_result(httpd_req_t *r, const cJSON *id) {
    // The directory is already validated static JSON. Avoid constructing its
    // entire cJSON tree and growing a second serialization buffer per request.
    static const char prefix[] = "{\"jsonrpc\":\"2.0\",\"id\":";
    static const char middle[] = ",\"result\":";
    char *encoded = cJSON_PrintUnformatted(id);
    if (!encoded)
        return ESP_ERR_NO_MEM;
    size_t id_size = strlen(encoded);
    size_t size = sizeof(prefix)-1 + id_size + sizeof(middle)-1 + sizeof(mcp_tools_json)-1 + 1;
    char *body = malloc(size + 1);
    if (!body) {
        cJSON_free(encoded);
        return ESP_ERR_NO_MEM;
    }
    size_t at = 0;
    memcpy(body+at, prefix, sizeof(prefix)-1);at += sizeof(prefix)-1;
    memcpy(body+at, encoded, id_size);at += id_size;
    memcpy(body+at, middle, sizeof(middle)-1);at += sizeof(middle)-1;
    memcpy(body+at, mcp_tools_json, sizeof(mcp_tools_json)-1);at += sizeof(mcp_tools_json)-1;
    memcpy(body+at, "}", 2);
    cJSON_free(encoded);
    response_headers(r);
    httpd_resp_set_status(r, "200 OK");
    httpd_resp_set_type(r, "application/json");
    esp_err_t result = httpd_resp_send(r, body, size);
    mbedtls_platform_zeroize(body, size);
    free(body);
    return result;
}
static esp_err_t tool_result(httpd_req_t *r, const cJSON *id, bool success, const char *text,
                             cJSON *data) {
    cJSON *result = cJSON_CreateObject(), *content = cJSON_AddArrayToObject(result, "content");
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "type", "text");
    cJSON_AddStringToObject(item, "text", text);
    cJSON_AddItemToArray(content, item);
    cJSON_AddBoolToObject(result, "isError", !success);
    if (data)
        cJSON_AddItemToObject(result, "structuredContent", data);
    return rpc_result(r, id, result);
}
static bool header(httpd_req_t *r, const char *name, char *out, size_t size) {
    size_t len = httpd_req_get_hdr_value_len(r, name);
    return len > 0 && len < size && httpd_req_get_hdr_value_str(r, name, out, size) == ESP_OK;
}
// IDF HTTP listens on a dual-stack IPv6 socket when LWIP_IPV6 is enabled.
// Accept the same exact USB IPv4 addresses in either sockaddr representation;
// native IPv6 and all other interfaces/peers remain outside this service.
static bool socket_address_matches(const struct sockaddr_storage *address, socklen_t length,
                                   uint32_t expected) {
    if (address->ss_family == AF_INET && length >= sizeof(struct sockaddr_in))
        return ((const struct sockaddr_in *)address)->sin_addr.s_addr == expected;
#if LWIP_IPV6
    if (address->ss_family == AF_INET6 && length >= sizeof(struct sockaddr_in6)) {
        const struct in6_addr *ip = &((const struct sockaddr_in6 *)address)->sin6_addr;
        return IN6_IS_ADDR_V4MAPPED(ip) && !memcmp(ip->s6_addr + 12, &expected, sizeof(expected));
    }
#endif
    return false;
}
static const char *usb_scope_error(httpd_req_t *r, bool secure) {
    int fd = httpd_req_to_sockfd(r);
    struct sockaddr_storage local = {0}, peer = {0};
    socklen_t len = sizeof(local);
    if (getsockname(fd, (struct sockaddr *)&local, &len) ||
        !socket_address_matches(&local, len, inet_addr("192.168.7.1")))
        return "USB device address required";
    len = sizeof(peer);
    if (getpeername(fd, (struct sockaddr *)&peer, &len) ||
        !socket_address_matches(&peer, len, inet_addr("192.168.7.2")))
        return "USB phone address 192.168.7.2 required";
    char host[160],origin[160];
    if(!header(r,"Host",host,sizeof(host)))return "Host required";
    bool has_origin=httpd_req_get_hdr_value_len(r,"Origin")!=0;
    if(has_origin&&!header(r,"Origin",origin,sizeof(origin)))return "Origin invalid";
    if(!network_authority(host,has_origin?origin:NULL,"192.168.7.1",secure,!discovery_conflict()))
        return "USB Host/Origin mismatch";
    return NULL;
}
static uint32_t socket_ipv4(const struct sockaddr_storage *address, socklen_t length) {
    if (address->ss_family==AF_INET && length>=sizeof(struct sockaddr_in))
        return ((const struct sockaddr_in *)address)->sin_addr.s_addr;
#if LWIP_IPV6
    if (address->ss_family==AF_INET6 && length>=sizeof(struct sockaddr_in6)) {
        const struct in6_addr *ip=&((const struct sockaddr_in6 *)address)->sin6_addr;
        uint32_t value=0;
        if (IN6_IS_ADDR_V4MAPPED(ip)) memcpy(&value,ip->s6_addr+12,4);
        return value;
    }
#endif
    return 0;
}
static bool ap_request(httpd_req_t *r);
static bool wifi_request(httpd_req_t *r) {
    struct sockaddr_storage local={0}; socklen_t length=sizeof(local);
    return !getsockname(httpd_req_to_sockfd(r),(struct sockaddr *)&local,&length) &&
           socket_ipv4(&local,length)!=inet_addr("192.168.7.1");
}
static const char *request_scope_error(httpd_req_t *r,bool secure) {
    if (!wifi_request(r)) return usb_scope_error(r,secure);
    if (secure) return "HTTPS is available on USB only";
    wireless_state network=wireless_snapshot();
    int fd=httpd_req_to_sockfd(r);
    struct sockaddr_storage local={0},peer={0};socklen_t length=sizeof(local);
    if(getsockname(fd,(struct sockaddr *)&local,&length))return "Local device address required";
    uint32_t address=socket_ipv4(&local,length),mask=network.netmask;
    bool ap=network.setup_ap&&address==network.ap_address;
    if(ap)mask=inet_addr("255.255.255.0");
    else if(!network.connected||!network.address||address!=network.address||!mask)return "Wi-Fi unavailable";
    length=sizeof(peer);
    if(getpeername(fd,(struct sockaddr *)&peer,&length))return "Local Wi-Fi peer required";
    uint32_t remote=socket_ipv4(&peer,length);
    if(!remote||remote==address||(remote&mask)!=(address&mask)||(remote&~mask)==0||(remote&~mask)==~mask)
        return "Same-subnet Wi-Fi peer required";
    char ip[16],host[160],origin[160];
    snprintf(ip,sizeof(ip),"%s",inet_ntoa((struct in_addr){.s_addr=address}));
    if(!header(r,"Host",host,sizeof(host)))return "Host required";
    bool has_origin=httpd_req_get_hdr_value_len(r,"Origin")!=0;
    if(has_origin&&!header(r,"Origin",origin,sizeof(origin)))return "Origin invalid";
    if(!network_authority(host,has_origin?origin:NULL,ip,false,!discovery_conflict()))return "Wi-Fi Host/Origin mismatch";
    return NULL;
}
static bool bearer(httpd_req_t *r) {
    char token[80];
    xSemaphoreTake(lock, portMAX_DELAY);
    bool ok = !atomic_load(&identity_busy) && identity_ready() &&
              header(r, "Authorization", token, sizeof(token)) && strlen(token) == 71 &&
              !memcmp(token, "Bearer ", 7) && equal_secret(token + 7, identity_token(), 64);
    xSemaphoreGive(lock);
    mbedtls_platform_zeroize(token, sizeof(token));
    return ok;
}
static bool unique_objects(const cJSON *value, unsigned depth) {
    if (depth > 16)
        return false;
    for (const cJSON *a = value->child; a; a = a->next) {
        if (cJSON_IsObject(value))
            for (const cJSON *b = a->next; b; b = b->next)
                if (a->string && b->string && !strcmp(a->string, b->string))
                    return false;
        if (!unique_objects(a, depth + 1))
            return false;
    }
    return true;
}
// Reject excessive nesting before allocation, embedded NUL and invalid UTF-8.
static bool valid_body(const unsigned char *s, size_t length) {
    unsigned depth = 0;
    bool quoted = false, escaped = false;
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = s[i];
        if (!c)
            return false;
        if (c >= 0x80) {
            unsigned count;
            uint32_t code;
            if (c >= 0xc2 && c <= 0xdf) {
                count = 1;
                code = c & 31;
            } else if (c >= 0xe0 && c <= 0xef) {
                count = 2;
                code = c & 15;
            } else if (c >= 0xf0 && c <= 0xf4) {
                count = 3;
                code = c & 7;
            } else
                return false;
            unsigned bytes = count;
            while (count--) {
                if (++i >= length || (s[i] & 0xc0) != 0x80)
                    return false;
                code = (code << 6) | (s[i] & 63);
            }
            if ((bytes == 2 && code < 0x800) || (bytes == 3 && code < 0x10000) || code > 0x10ffff ||
                (code >= 0xd800 && code <= 0xdfff))
                return false;
            continue;
        }
        if (quoted) {
            if (escaped) {
                if (c == 'u' && i + 4 < length && !memcmp(s + i + 1, "0000", 4))
                    return false;
                escaped = false;
            } else if (c == '\\')
                escaped = true;
            else if (c == '"')
                quoted = false;
        } else if (c == '"')
            quoted = true;
        else if (c == '{' || c == '[') {
            if (++depth > 16)
                return false;
        } else if (c == '}' || c == ']') {
            if (!depth)
                return false;
            --depth;
        }
    }
    return !depth && !quoted && !escaped;
}
static cJSON *read_json(httpd_req_t *r) {
    char type[80];
    if (r->content_len == 0 || r->content_len > BODY_LIMIT ||
        !header(r, "Content-Type", type, sizeof(type)) ||
        (strcmp(type, "application/json") && strncmp(type, "application/json;", 17)))
        return NULL;
    char *body = malloc(r->content_len + 1);
    if (!body)
        return NULL;
    size_t used = 0;
    while (used < r->content_len) {
        int got = httpd_req_recv(r, body + used, r->content_len - used);
        if (got <= 0) {
            free(body);
            return NULL;
        }
        used += got;
    }
    body[used] = 0;
    cJSON *parsed = NULL;
    if (valid_body((unsigned char *)body, used))
        parsed = cJSON_ParseWithLengthOpts(body, used + 1, NULL, true);
    mbedtls_platform_zeroize(body, used);
    free(body);
    if (parsed && !unique_objects(parsed, 0)) {
        cJSON_Delete(parsed);
        parsed = NULL;
    }
    return parsed;
}
static bool object_keys(const cJSON *o, const char *const *keys, size_t count) {
    if (!cJSON_IsObject(o))
        return false;
    for (const cJSON *item = o->child; item; item = item->next) {
        bool found = false;
        for (size_t i = 0; i < count; ++i)
            if (!strcmp(item->string, keys[i]))
                found = true;
        if (!found)
            return false;
    }
    return true;
}
static bool empty_object(const cJSON *o) { return !o || (cJSON_IsObject(o) && !o->child); }
static bool metadata_only(const cJSON *params) {
    const char *keys[] = {"_meta"};
    return !params || (object_keys(params, keys, 1) &&
                       (!field(params, "_meta") || cJSON_IsObject(field(params, "_meta"))));
}
static bool integer(const cJSON *v, double min, double max) {
    return cJSON_IsNumber(v) && isfinite(v->valuedouble) && v->valuedouble >= min &&
           v->valuedouble <= max && floor(v->valuedouble) == v->valuedouble;
}
static bool valid_id(const cJSON *id) {
    return (cJSON_IsString(id) && strlen(id->valuestring) > 0 && strlen(id->valuestring) <= 64) ||
           integer(id, -9007199254740991.0, 9007199254740991.0);
}
static session *find_session(const char *id) {
    for (unsigned i = 0; i < SESSION_LIMIT; ++i) {
        if (sessions[i].id[0] && (uint32_t)(now_ms() - sessions[i].touched) >= SESSION_TTL)
            memset(&sessions[i], 0, sizeof(session));
        if (sessions[i].id[0] && !strcmp(id, sessions[i].id))
            return &sessions[i];
    }
    return NULL;
}
static bool same_id(const cJSON *a, const cJSON *b) {
    if (cJSON_IsString(a) && cJSON_IsString(b))
        return !strcmp(a->valuestring, b->valuestring);
    return cJSON_IsNumber(a) && cJSON_IsNumber(b) && a->valuedouble == b->valuedouble;
}
void mcp_stop_input(void) {
    atomic_fetch_add(&input_generation, 1);
    actions_request_release();
}
void mcp_network_lost(void) {
    if (!lock) return;
    xSemaphoreTake(lock,portMAX_DELAY);
    bool owned=owner_wifi && owner[0] && owner_generation==device_generation();
    if (active && active->wifi) atomic_store(&active->cancelled,true);
    xSemaphoreGive(lock);
    if (owned) device_stop();
}
bool mcp_ready(void) {
    return !atomic_load(&identity_busy) &&
           (atomic_load(&http_running) || atomic_load(&https_running));
}
bool mcp_https_ready(void) { return atomic_load(&https_running); }
bool mcp_http_ready(void) { return atomic_load(&http_running); }
int mcp_http_error(void) { return atomic_load(&http_start_error); }
static bool input_alive(uint32_t unused, void *context) {
    (void)unused;
    input_request *p = context;
    if (p->wifi) {
        wireless_state network=wireless_snapshot();
        unsigned char byte;
        int received=recv(p->socket,&byte,1,MSG_PEEK|MSG_DONTWAIT);
        if (!network.connected || network.generation!=p->network_generation || received==0 ||
            (received<0 && errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)) {
            device_stop(); return false;
        }
    }
    return !atomic_load(&p->cancelled) && p->generation == atomic_load(&input_generation) &&
           p->device_generation == device_generation() && device_control_allowed() &&
           within(p->deadline);
}
static void input_done(const action *a, bool result, void *context) {
    input_request *p = context;
    p->success = result;
    p->completed_units = a->completed_units;
    xSemaphoreGive(p->done);
}
static void respond_input(void *context) {
    input_request *p = context;
    xSemaphoreTake(p->done, portMAX_DELAY);
    cJSON *progress = NULL;
    char message[240];
    if (p->input_kind >= ACTION_KEYBOARD) {
        const char *unit = hid_input_unit(p->input_kind);
        progress = cJSON_CreateObject();
        cJSON_AddBoolToObject(progress,"sequenceCompleted",p->success);
        cJSON_AddNumberToObject(progress,"completedUnits",p->completed_units);
        cJSON_AddNumberToObject(progress,"totalUnits",p->total_units);
        cJSON_AddStringToObject(progress,"unit",unit);
        cJSON_AddStringToObject(progress,"phoneEffect","unverified");
        cJSON_AddBoolToObject(progress,"partialEffectsPossible",!p->success);
        snprintf(message,sizeof(message),"HID %u/%u %s completed. Phone effect is unverified. %s",
                 p->completed_units,p->total_units,unit,
                 p->success ? "Observe the phone to verify the result."
                            : "Input interrupted or failed; partial effects are possible. Observe; do not retry.");
    }
    tool_result(p->req, p->id, p->success,
                progress ? message : p->success ? "HID sequence completed. Observe the phone to verify the result."
                           : "Input interrupted or failed; partial effects are possible. Observe "
                             "the phone. Do not automatically retry.",
                progress);
    httpd_req_async_handler_complete(p->req);
    xSemaphoreTake(lock, portMAX_DELAY);
    active = NULL;
    xSemaphoreGive(lock);
    cJSON_Delete(p->id);
    vSemaphoreDelete(p->done);
    free(p);
    vTaskDelete(NULL);
}
static bool parse_input(const char *name, const cJSON *args, action *out) {
    memset(out, 0, sizeof(*out));
    if (!strcmp(name, "key")) {
        const char *keys[] = {"keycode", "modifiers"};
        out->kind = ACTION_KEY;
        if (!object_keys(args, keys, 2) || !integer(field(args, "keycode"), 4, 231) ||
            !integer(field(args, "modifiers"), 0, 255))
            return false;
        out->key = field(args, "keycode")->valuedouble;
        out->modifiers = field(args, "modifiers")->valuedouble;
        return true;
    }
    if (!strcmp(name, "type_ascii")) {
        const char *keys[] = {"text"};
        out->kind = ACTION_TYPE;
        const cJSON *text = field(args, "text");
        if (!object_keys(args, keys, 1) || !cJSON_IsString(text))
            return false;
        size_t length = strlen(text->valuestring);
        if (!length || length > 120)
            return false;
        for (size_t i = 0; i < length; ++i) {
            uint8_t key, mods;
            if (!ascii_key(text->valuestring[i], &key, &mods))
                return false;
        }
        memcpy(out->text, text->valuestring, length);
        out->text_length = length;
        return true;
    }
    return hid_input_parse(name, args, out);
}
static esp_err_t dispatch_input(httpd_req_t *r, const cJSON *id, const char *sid, action *a) {
    xSemaphoreTake(lock, portMAX_DELAY);
    uint32_t gen = device_generation();
    if (!device_control_allowed() || active ||
        (owner_generation == gen && owner[0] && strcmp(owner, sid)) || !actions_reserve()) {
        xSemaphoreGive(lock);
        return tool_result(r, id, false,
                           "Input rejected: select MCP, physically enable control, and use one "
                           "session with no action in progress.",
                           NULL);
    }
    // Consume each input ID once for the entire session. Bound the ledger;
    // a full session must be replaced, never silently forget IDs and replay.
    session *s = find_session(sid);
    char *encoded_id = cJSON_PrintUnformatted(id);
    unsigned char digest[32];
    bool remembered =
        s && encoded_id &&
        mbedtls_sha256((unsigned char *)encoded_id, strlen(encoded_id), digest, 0) == 0;
    bool repeated = false;
    if (remembered)
        for (unsigned i = 0; i < s->input_ids_count; ++i)
            if (!memcmp(s->input_ids[i], digest, 32))
                repeated = true;
    cJSON_free(encoded_id);
    if (!remembered || repeated || s->input_ids_count == INPUT_IDS_LIMIT) {
        actions_unreserve();
        xSemaphoreGive(lock);
        return tool_result(
            r, id, false,
            "Input rejected: repeated request ID or session input limit. Use fresh IDs; create a "
            "new session and physically re-enable control when the limit is reached.",
            NULL);
    }
    memcpy(s->input_ids[s->input_ids_count++], digest, 32);
    input_request *p = calloc(1, sizeof(*p));
    if (p) {
        p->done = xSemaphoreCreateBinary();
        p->id = cJSON_Duplicate(id, true);
    }
    if (!p || !p->done || !p->id || httpd_req_async_handler_begin(r, &p->req) != ESP_OK) {
        // Release the reservation without introducing a fake HID action.
        actions_unreserve();
        if (p) {
            if (p->done)
                vSemaphoreDelete(p->done);
            cJSON_Delete(p->id);
            free(p);
        }
        xSemaphoreGive(lock);
        return tool_result(r, id, false, "Input rejected: resources unavailable.", NULL);
    }
    p->generation = atomic_load(&input_generation);
    p->wifi = wifi_request(r);
    p->network_generation = wireless_snapshot().generation;
    p->socket = httpd_req_to_sockfd(r);
    p->device_generation = gen;
    p->deadline = now_ms() + 8000;
    p->input_kind = a->kind;
    p->total_units = hid_input_total(a);
    snprintf(p->session_id, sizeof(p->session_id), "%s", sid);
    active = p;
    snprintf(owner, sizeof(owner), "%s", sid);
    owner_generation = gen;
    owner_wifi = p->wifi;
    if (xTaskCreate(respond_input, "mcp-result", 6144, p, 3, NULL) != pdPASS) {
        active = NULL;
        actions_unreserve();
        httpd_req_async_handler_complete(p->req);
        vSemaphoreDelete(p->done);
        cJSON_Delete(p->id);
        free(p);
        xSemaphoreGive(lock);
        return tool_result(r, id, false, "Input rejected: responder unavailable.", NULL);
    }
    if (!actions_enqueue(a, input_alive, input_done, p)) {
        p->success = false;
        xSemaphoreGive(p->done);
    }
    xSemaphoreGive(lock);
    return ESP_OK;
}
static esp_err_t handle_call(httpd_req_t *r, const cJSON *id, const char *sid,
                             const cJSON *params) {
    const char *keys[] = {"name", "arguments", "_meta"};
    const cJSON *name = field(params, "name"), *args = field(params, "arguments");
    if (!object_keys(params, keys, 3) || !cJSON_IsString(name) ||
        (field(params, "_meta") && !cJSON_IsObject(field(params, "_meta"))))
        return rpc_error(r, id, -32602, "Invalid tool call parameters");
    if (!strcmp(name->valuestring, "device_status")) {
        if (!empty_object(args))
            return rpc_error(r, id, -32602, "device_status takes no arguments");
        cJSON *data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "mode", device_mode() == DEVICE_MCP ? "mcp" : "setup");
        cJSON_AddBoolToObject(data, "usbOnline", bridge_usb_online());
        cJSON_AddStringToObject(data,"inputTransport",bridge_input_name());
        cJSON_AddBoolToObject(data,"inputOnline",bridge_input_online());
        cJSON_AddBoolToObject(data,"bleConnected",ble_input_connected());
        cJSON_AddBoolToObject(data,"bleReady",ble_input_online());
        wireless_state network=wireless_snapshot();
        cJSON_AddBoolToObject(data,"wifiConnected",network.connected);
        char address[16]; wireless_address(address,sizeof(address));
        cJSON_AddStringToObject(data,"wifiAddress",address);
        cJSON_AddBoolToObject(data, "inputBusy", actions_busy());
        bool enabled = device_control_allowed();
        cJSON_AddBoolToObject(data, "controlEnabled", enabled);
        // Keep the status field: null means the enabled grant has no time limit.
        if (enabled)
            cJSON_AddNullToObject(data, "controlSecondsRemaining");
        else
            cJSON_AddNumberToObject(data, "controlSecondsRemaining", 0);
        return tool_result(r, id, true,
                           "Device state only. Screen observation belongs to your client.", data);
    }
    if (!strcmp(name->valuestring, "stop")) {
        if (!empty_object(args))
            return rpc_error(r, id, -32602, "stop takes no arguments");
        device_stop();
        return tool_result(r, id, true,
                           "Control revoked; release requested. Previous input cannot be undone.",
                           NULL);
    }
    if (strcmp(name->valuestring, "tap") && strcmp(name->valuestring, "drag") &&
        strcmp(name->valuestring, "key") && strcmp(name->valuestring, "type_ascii") &&
        !hid_input_is_tool(name->valuestring))
        return rpc_error(r, id, -32602, "Unknown tool");
    action a;
    if (!parse_input(name->valuestring, args, &a)) {
        if (hid_input_is_tool(name->valuestring)) {
            char reason[240];
            snprintf(reason,sizeof(reason),
                     "Invalid %s arguments; no input dispatched. Follow tools/list. "
                     "Unlisted compatibility calls are documented in docs/hid-tools.md.",
                     name->valuestring);
            return rpc_error(r,id,-32602,reason);
        }
        return rpc_error(r, id, -32602, "Invalid input arguments");
    }
    return dispatch_input(r, id, sid, &a);
}
static esp_err_t mcp_handler(httpd_req_t *r) {
    bool secure = r->user_ctx != NULL;
    if(ap_request(r))return http_error(r,"403 Forbidden","Setup hotspot is configuration-only");
    const char *scope_error = request_scope_error(r, secure);
    if (scope_error)
        return http_error(r, "403 Forbidden", scope_error);
    if (!bearer(r)) {
        httpd_resp_set_hdr(r, "WWW-Authenticate", "Bearer");
        return http_error(r, "401 Unauthorized", "Bearer credential required");
    }
    if (r->method == HTTP_GET) {
        httpd_resp_set_hdr(r, "Allow", "POST, DELETE");
        return empty_response(r, "405 Method Not Allowed");
    }
    char sid[40] = {0};
    bool has_sid = header(r, "MCP-Session-Id", sid, sizeof(sid));
    if (r->method == HTTP_DELETE) {
        xSemaphoreTake(lock, portMAX_DELAY);
        session *s = has_sid ? find_session(sid) : NULL;
        bool found = s != NULL && s->wifi==wifi_request(r) &&
                     (!s->wifi || s->network_generation==wireless_snapshot().generation);
        if (found)
            memset(s, 0, sizeof(*s));
        if (found && active && !strcmp(active->session_id, sid))
            atomic_store(&active->cancelled, true);
        bool owned = found && owner[0] && !strcmp(owner, sid);
        xSemaphoreGive(lock);
        if (owned)
            device_stop();
        return found ? empty_response(r, "200 OK")
                     : http_error(r, "404 Not Found", "Unknown session");
    }
    if (r->content_len > BODY_LIMIT)
        return http_error(r, "413 Content Too Large", "Request exceeds limit");
    char accept[180];
    if (!header(r, "Accept", accept, sizeof(accept)) || !strstr(accept, "application/json"))
        return http_error(r, "406 Not Acceptable", "Accept application/json required");
    cJSON *root = read_json(r);
    if (!root)
        return rpc_error(r, NULL, -32700, "Invalid or oversized JSON body");
    const cJSON *id = field(root, "id"), *method = field(root, "method"),
                *params = field(root, "params");
    const char *keys[] = {"jsonrpc", "id", "method", "params"};
    if (!object_keys(root, keys, 4) || !string_is(field(root, "jsonrpc"), "2.0") ||
        !cJSON_IsString(method) || (id && !valid_id(id)) || (params && !cJSON_IsObject(params))) {
        esp_err_t result =
            rpc_error(r, valid_id(id) ? id : NULL, -32600, "Invalid JSON-RPC request");
        cJSON_Delete(root);
        return result;
    }
    esp_err_t result;
    if (!strcmp(method->valuestring, "initialize")) {
        const cJSON *version = field(params, "protocolVersion"),
                    *caps = field(params, "capabilities"),
                    *client_info = field(params, "clientInfo");
        if (!id || !cJSON_IsString(version) || !cJSON_IsObject(caps) ||
            !cJSON_IsObject(client_info) || has_sid) {
            result = rpc_error(r, id, -32602, "Invalid initialize request");
        } else {
            xSemaphoreTake(lock, portMAX_DELAY);
            find_session("");
            session *s = NULL;
            for (unsigned i = 0; i < SESSION_LIMIT; ++i)
                if (!sessions[i].id[0]) {
                    s = &sessions[i];
                    break;
                }
            const char *session_error = s ? "Random source unavailable" : "Session limit reached";
            if (s && identity_random_hex(s->id, 16)) {
                s->touched = now_ms();
                s->initialized = false;
                s->wifi=wifi_request(r);
                s->network_generation=wireless_snapshot().generation;
                snprintf(sid, sizeof(sid), "%s", s->id);
            } else
                s = NULL;
            xSemaphoreGive(lock);
            if (!s)
                result = http_error(r, "503 Service Unavailable", session_error);
            else {
                httpd_resp_set_hdr(r, "MCP-Session-Id", sid);
                cJSON *data = cJSON_CreateObject();
                cJSON_AddStringToObject(data, "protocolVersion", VERSION);
                cJSON *cap = cJSON_AddObjectToObject(data, "capabilities");
                cJSON_AddObjectToObject(cap, "tools");
                cJSON *info = cJSON_AddObjectToObject(data, "serverInfo");
                cJSON_AddStringToObject(info, "name", "stopwatch");
                cJSON_AddStringToObject(info, "version", esp_app_get_description()->version);
                cJSON_AddStringToObject(info, "description",
                                        "iPhone USB/BLE HID: open apps via icons/Spotlight, navigate, type ASCII, edit, draw, scroll and medi"
                                        "a keys. 打开应用 搜索 输入 绘图 滚动. Seven complementary tools. When the overview is not visible, consider brow"
                                        "sing this group with tool_search: empty query, exact client group, limit=10. Read selected schemas b"
                                        "efore calls. Physical enable required. Client observes; no UI tree or guaranteed app success. No aut"
                                        "omatic replay. BLE experimental; Full Keyboard Access optional.");
                cJSON_AddStringToObject(data, "instructions",
                                        "Input requires StopWatch MCP mode and physical enable. Coordinates: portrait 0..1, origin top-left. "
                                        "Each call releases held input. HID completion is not app success; observe separately. Interrupted/lo"
                                        "st replies may have partial effects; never replay automatically. No custom phone bindings; Full Keyb"
                                        "oard Access is optional, not assumed. Refresh tools after firmware updates. For multi-step control, "
                                        "consider browsing the whole tool group if its overview is not visible: tool_search with empty query,"
                                        " exact client group and limit=10; follow nextOffset if needed. Search summaries are not full schemas"
                                        "; describe selected tools before calls.");
                result = rpc_result(r, id, data);
            }
        }
    } else {
        char version[32];
        bool version_ok = header(r, "MCP-Protocol-Version", version, sizeof(version)) &&
                          !strcmp(version, VERSION);
        xSemaphoreTake(lock, portMAX_DELAY);
        session *s = has_sid ? find_session(sid) : NULL;
        bool valid = s != NULL && s->wifi==wifi_request(r) &&
                     (!s->wifi || s->network_generation==wireless_snapshot().generation);
        bool initialized = valid && s->initialized;
        if (valid && version_ok) {
            s->touched = now_ms();
            if (!strcmp(method->valuestring, "notifications/initialized") && !id &&
                metadata_only(params))
                s->initialized = true;
        }
        xSemaphoreGive(lock);
        if (!valid)
            result = http_error(r, "404 Not Found", "Unknown or expired MCP session");
        else if (!version_ok)
            result = http_error(r, "400 Bad Request", "MCP-Protocol-Version must be 2025-11-25");
        else if (!id) {
            if (!strcmp(method->valuestring, "notifications/cancelled") && params) {
                const cJSON *request_id = field(params, "requestId");
                xSemaphoreTake(lock, portMAX_DELAY);
                if (active && !strcmp(active->session_id, sid) && same_id(active->id, request_id))
                    atomic_store(&active->cancelled, true);
                xSemaphoreGive(lock);
            }
            result = empty_response(r, "202 Accepted");
        } else if (!strcmp(method->valuestring, "ping") && metadata_only(params))
            result = rpc_result(r, id, cJSON_CreateObject());
        else if (!initialized)
            result = rpc_error(r, id, -32600, "Send notifications/initialized first");
        else if (!strcmp(method->valuestring, "tools/list") && metadata_only(params))
            result = rpc_tools_result(r, id);
        else if (!strcmp(method->valuestring, "tools/call"))
            result = handle_call(r, id, sid, params);
        else
            result = rpc_error(r, id, -32601, "Method not supported");
    }
    cJSON_Delete(root);
    return result;
}
static bool setup_valid_locked(void) {
    return device_mode() == DEVICE_SETUP && within(setup_until);
}
static bool setup_cookie(httpd_req_t *r) {
    char value[100];
    if (!header(r, "Cookie", value, sizeof(value)))
        return false;
    // This service sets only one setup cookie. Do not accept substring matches.
    char expected[50];
    snprintf(expected, sizeof(expected), "sw_setup=%s", setup_nonce);
    return setup_nonce[0] && !strcmp(value, expected);
}
void mcp_open_setup(void) {
    xSemaphoreTake(lock, portMAX_DELAY);
    ++setup_generation;
    setup_until = now_ms() + SETUP_TTL;
    setup_nonce[0] = 0;
    setup_code[0] = 0;
    pair_state = PAIR_NONE;
    xSemaphoreGive(lock);
}
void mcp_close_setup(void) {
    xSemaphoreTake(lock, portMAX_DELAY);
    ++setup_generation;
    setup_until = 0;
    setup_nonce[0] = 0;
    pair_state = PAIR_NONE;
    xSemaphoreGive(lock);
}
bool mcp_setup_pending(char *code, size_t size) {
    xSemaphoreTake(lock, portMAX_DELAY);
    bool pending = setup_valid_locked() && pair_state == PAIR_PENDING;
    if (pending)
        snprintf(code, size, "%s", setup_code);
    xSemaphoreGive(lock);
    return pending;
}
mcp_pair_state mcp_pair_snapshot(char *code, size_t code_size, char *fingerprint,
                                 size_t fingerprint_size) {
    xSemaphoreTake(lock, portMAX_DELAY);
    mcp_pair_state state = !setup_valid_locked()        ? MCP_PAIR_EXPIRED
                           : pair_state == PAIR_PENDING ? MCP_PAIR_PENDING
                           : pair_state == PAIR_WORKING ? MCP_PAIR_WORKING
                           : pair_state == PAIR_READY && !atomic_load(&identity_busy)
                               ? MCP_PAIR_READY
                           : pair_state == PAIR_READY  ? MCP_PAIR_WORKING
                           : pair_state == PAIR_FAILED ? MCP_PAIR_FAILED
                                                       : MCP_PAIR_IDLE;
    if (code_size)
        snprintf(code, code_size, "%s", state == MCP_PAIR_PENDING ? setup_code : "");
    if (fingerprint_size)
        snprintf(fingerprint, fingerprint_size, "%s",
                 state == MCP_PAIR_READY ? identity_fingerprint() : "");
    xSemaphoreGive(lock);
    return state;
}
void mcp_confirm_setup(void) {
    xSemaphoreTake(lock, portMAX_DELAY);
    if (setup_valid_locked() && pair_state == PAIR_PENDING) {
        identity_job job = {.now = setup_time, .setup_generation = setup_generation};
        if (xQueueSend(identity_jobs, &job, 0) == pdTRUE)
            pair_state = PAIR_WORKING;
    }
    xSemaphoreGive(lock);
}
bool mcp_reset_identity(void) {
    mcp_close_setup();
    identity_job job = {.reset = true};
    return xQueueSend(identity_jobs, &job, 0) == pdTRUE;
}
static void add_memory_diagnostics(cJSON *data) {
    const unsigned internal = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
    const unsigned external = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
    bool psram_ready = false;
    size_t psram_size = 0;
#if CONFIG_SPIRAM
    psram_ready = esp_psram_is_initialized();
    if (psram_ready) psram_size = esp_psram_get_size();
#endif
    // Retain total-heap fields; capacity in PSRAM cannot replace internal blocks.
    cJSON_AddNumberToObject(data, "freeHeapBytes", esp_get_free_heap_size());
    cJSON_AddNumberToObject(data, "minimumFreeHeapBytes", esp_get_minimum_free_heap_size());
    cJSON_AddNumberToObject(data, "freeInternalHeapBytes", heap_caps_get_free_size(internal));
    cJSON_AddNumberToObject(data, "minimumFreeInternalHeapBytes", heap_caps_get_minimum_free_size(internal));
    cJSON_AddNumberToObject(data, "largestFreeInternalBlockBytes", heap_caps_get_largest_free_block(internal));
    cJSON_AddBoolToObject(data, "psramInitialized", psram_ready);
    cJSON_AddNumberToObject(data, "psramSizeBytes", psram_size);
    cJSON_AddNumberToObject(data, "freePsramBytes", heap_caps_get_free_size(external));
    cJSON_AddNumberToObject(data, "minimumFreePsramBytes", heap_caps_get_minimum_free_size(external));
    cJSON_AddNumberToObject(data, "largestFreePsramBlockBytes", heap_caps_get_largest_free_block(external));
}
static esp_err_t diagnostic_handler(httpd_req_t *r) {
    const char *scope_error = request_scope_error(r, false);
    if (scope_error)
        return http_error(r, "403 Forbidden", scope_error);
    // Read-only startup diagnostics: no identities, inputs or pairing secrets.
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "firmwareVersion", esp_app_get_description()->version);
    cJSON_AddNumberToObject(data, "resetReason", esp_reset_reason());
    cJSON_AddNumberToObject(data, "uptimeSeconds", now_ms()/1000);
    cJSON_AddStringToObject(data, "displayStage", display_status());
    cJSON_AddStringToObject(data, "language", device_ui_language());
    cJSON_AddBoolToObject(data, "touchReady", touch_ready());
    cJSON_AddNumberToObject(data, "touchError", touch_error());
    cJSON_AddNumberToObject(data, "displayError", display_error());
    cJSON_AddBoolToObject(data, "usbOnline", bridge_usb_online());
    cJSON_AddBoolToObject(data, "httpMCPReady", atomic_load(&http_running));
    cJSON_AddNumberToObject(data, "httpStartupError", mcp_http_error());
    add_memory_diagnostics(data);
    reset_trace_info previous_ap = reset_trace_previous();
    cJSON_AddBoolToObject(data, "previousAPTraceReady", previous_ap.valid);
    if (previous_ap.valid) {
        cJSON_AddNumberToObject(data, "previousAPStage", previous_ap.stage);
        cJSON_AddNumberToObject(data, "previousAPError", previous_ap.error);
        cJSON_AddNumberToObject(data, "previousAPFreeInternalBytes", previous_ap.free_internal);
        cJSON_AddNumberToObject(data, "previousAPLargestInternalBlockBytes", previous_ap.largest_internal);
        cJSON_AddNumberToObject(data, "previousAPStackFreeBytes", previous_ap.stack_free);
    }
    // Diagnostic occupancy only; do not block status on a busy MCP operation.
    bool session_stats = xSemaphoreTake(lock, 0) == pdTRUE;
    unsigned allocated = 0, initialized = 0;
    if (session_stats) {
        for (unsigned i = 0; i < SESSION_LIMIT; ++i) {
            if (sessions[i].id[0]) {
                ++allocated;
                initialized += sessions[i].initialized;
            }
        }
        xSemaphoreGive(lock);
    }
    cJSON_AddBoolToObject(data, "mcpSessionStatsReady", session_stats);
    if (session_stats) {
        cJSON_AddNumberToObject(data, "mcpSessionsAllocated", allocated);
        cJSON_AddNumberToObject(data, "mcpSessionsInitialized", initialized);
        cJSON_AddNumberToObject(data, "mcpSessionLimit", SESSION_LIMIT);
    }
    cJSON_AddBoolToObject(data, "httpsMCPReady", mcp_https_ready());
    cJSON_AddStringToObject(data,"inputTransport",bridge_input_name());
    cJSON_AddBoolToObject(data,"bleConnected",ble_input_connected());
    cJSON_AddBoolToObject(data,"bleReady",ble_input_online());
    cJSON_AddNumberToObject(data,"bleError",ble_input_error());
    wireless_state network=wireless_snapshot();
    cJSON_AddBoolToObject(data,"setupAP",network.setup_ap);
    cJSON_AddNumberToObject(data,"wifiSetupStage",network.setup_stage);
    cJSON_AddNumberToObject(data,"wifiSetupError",network.setup_error);
    cJSON_AddStringToObject(data,"mcpHost","stopwatch.local");
    cJSON_AddBoolToObject(data,"hostnameConflict",discovery_conflict());
    cJSON_AddBoolToObject(data,"wifiConfigured",network.configured);
    cJSON_AddBoolToObject(data,"wifiConnected",network.connected);
    cJSON_AddNumberToObject(data,"wifiError",network.error);
    char address[16]; wireless_address(address,sizeof(address));
    cJSON_AddStringToObject(data,"wifiAddress",address);
    return send_json(r, "200 OK", data);
}
static esp_err_t page_handler(httpd_req_t *r) {
    bool secure = r->user_ctx != NULL;
    const char *scope_error = request_scope_error(r, secure);
    if (scope_error)
        return http_error(r, "403 Forbidden", scope_error);
    response_headers(r);
    httpd_resp_set_type(r, "text/html; charset=utf-8");
    // Script is a fixed embedded asset; no user data is interpolated into HTML.
    httpd_resp_set_hdr(
        r, "Content-Security-Policy",
        "default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; connect-src "
        "'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'");
    return httpd_resp_send(r, (const char *)setup_html_start,
                           setup_html_end - setup_html_start - 1);
}
static esp_err_t enrollment_handler(httpd_req_t *r) {
    const char *scope_error = request_scope_error(r, false);
    if (scope_error)
        return http_error(r, "403 Forbidden", scope_error);
    cJSON *body = read_json(r);
    if (!body)
        return http_error(r, "400 Bad Request", "Invalid JSON");
    xSemaphoreTake(lock, portMAX_DELAY);
    if (!setup_valid_locked()) {
        xSemaphoreGive(lock);
        cJSON_Delete(body);
        return http_error(r, "403 Forbidden", "Select Connect MCP app in device settings first");
    }
    cJSON *reply = cJSON_CreateObject();
    const char *status = "200 OK";
    if (!strcmp(r->uri, "/enroll/start")) {
        const char *keys[] = {"time"};
        const cJSON *time = field(body, "time");
        if (!object_keys(body, keys, 1) || !integer(time, 1735689600, 4102444800.0) ||
            pair_state == PAIR_WORKING) {
            status = "400 Bad Request";
            cJSON_AddStringToObject(reply, "error", "Invalid time or enrollment in progress");
        } else if (pair_state == PAIR_PENDING || pair_state == PAIR_READY) {
            status = "409 Conflict";
            cJSON_AddStringToObject(reply, "error",
                                    "Setup already claimed; reopen Connect MCP app to restart");
        } else if (!identity_random_hex(setup_nonce, 16)) {
            status = "503 Service Unavailable";
            cJSON_AddStringToObject(reply, "error", "Random source unavailable");
        } else {
            unsigned value = 0;
            for (unsigned i = 0; i < 8; ++i)
                value = value * 16 +
                        (setup_nonce[i] <= '9' ? setup_nonce[i] - '0' : setup_nonce[i] - 'a' + 10);
            snprintf(setup_code, sizeof(setup_code), "%06u", value % 1000000);
            setup_time = (time_t)time->valuedouble;
            pair_state = PAIR_PENDING;
            cJSON_AddStringToObject(reply, "nonce", setup_nonce);
            cJSON_AddStringToObject(reply, "code", setup_code);
        }
    } else {
        const char *keys[] = {"nonce"};
        const cJSON *nonce = field(body, "nonce");
        if (!object_keys(body, keys, 1) || !cJSON_IsString(nonce) ||
            strlen(nonce->valuestring) != 32 ||
            !equal_secret(nonce->valuestring, setup_nonce, 32)) {
            status = "403 Forbidden";
            cJSON_AddStringToObject(reply, "error", "Invalid pairing proof");
        } else {
            bool ready = pair_state == PAIR_READY && !atomic_load(&identity_busy);
            cJSON_AddStringToObject(reply, "state",
                                    ready                       ? "ready"
                                    : pair_state == PAIR_FAILED ? "failed"
                                                                : "waiting");
            if (ready) {
                cJSON_AddStringToObject(reply, "fingerprint", identity_fingerprint());
                char cookie[100];
                snprintf(cookie, sizeof(cookie),
                         "sw_setup=%s; HttpOnly; SameSite=Strict; Path=/; Max-Age=600",
                         setup_nonce);
                httpd_resp_set_hdr(r, "Set-Cookie", cookie);
                // httpd stores header pointers until send; send while cookie is in scope.
                xSemaphoreGive(lock);
                cJSON_Delete(body);
                return send_json(r, status, reply);
            }
        }
    }
    xSemaphoreGive(lock);
    cJSON_Delete(body);
    return send_json(r, status, reply);
}
static esp_err_t config_handler(httpd_req_t *r) {
    bool secure = r->user_ctx != NULL;
    const char *scope_error = request_scope_error(r, secure);
    if (scope_error)
        return http_error(r, "403 Forbidden", scope_error);
    cJSON *args = read_json(r);
    const char *keys[]={"network"};
    const cJSON *selection=field(args,"network");
    bool empty=args && object_keys(args,keys,1) &&
               (!selection || string_is(selection,"usb") || string_is(selection,"wifi"));
    bool wifi=selection ? string_is(selection,"wifi") : wifi_request(r);
    cJSON_Delete(args);
    if (!empty)
        return http_error(r, "400 Bad Request", "Optional network must be usb or wifi");
    char address[32];
    if(!secure){
        if(discovery_conflict())return http_error(r,"409 Conflict","stopwatch.local name conflict; keep the numeric setup address and use only one StopWatch on this LAN");
        if(wifi&&!wireless_snapshot().configured)return http_error(r,"409 Conflict","Save Wi-Fi first");
        snprintf(address,sizeof(address),"stopwatch.local");
    }else snprintf(address,sizeof(address),"192.168.7.1");
    xSemaphoreTake(lock, portMAX_DELAY);
    bool allowed = setup_valid_locked() && pair_state == PAIR_READY &&
                   !atomic_load(&identity_busy) && identity_ready() && setup_cookie(r);
    cJSON *config = NULL;
    if (allowed) {
        config = cJSON_CreateObject();
        cJSON *servers = cJSON_AddObjectToObject(config, "mcpServers");
        cJSON *server = cJSON_AddObjectToObject(servers, "stopwatch");
        cJSON_AddStringToObject(server, "type", "http");
        char endpoint[64];
        snprintf(endpoint,sizeof(endpoint),secure && !wifi ? "https://%s:8443/mcp" : "http://%s/mcp",address);
        cJSON_AddStringToObject(server,"url",endpoint);
        cJSON *headers = cJSON_AddObjectToObject(server, "headers");
        char token[80];
        snprintf(token, sizeof(token), "Bearer %s", identity_token());
        cJSON_AddStringToObject(headers, "Authorization", token);
        mbedtls_platform_zeroize(token, sizeof(token));
    }
    xSemaphoreGive(lock);
    return allowed ? send_json(r, "200 OK", config)
                   : http_error(r, "403 Forbidden",
                                "Setup expired; pair from the initial phone setup page again");
}
static bool ap_request(httpd_req_t *r){
    struct sockaddr_storage local={0};socklen_t length=sizeof(local);
    wireless_state network=wireless_snapshot();
    return network.setup_ap&&!getsockname(httpd_req_to_sockfd(r),(struct sockaddr *)&local,&length)&&socket_ipv4(&local,length)==network.ap_address;
}
static const char *setup_transport_error(httpd_req_t *r){
    const char *error=request_scope_error(r,false);
    if(error)return error;
    return wifi_request(r)&&!ap_request(r)?"Use USB or the device setup hotspot to manage Wi-Fi":NULL;
}
static esp_err_t wifi_networks_handler(httpd_req_t *r){
    const char *error=setup_transport_error(r);if(error)return http_error(r,"403 Forbidden",error);
    xSemaphoreTake(lock,portMAX_DELAY);
    bool allowed=setup_valid_locked()&&pair_state==PAIR_READY&&!atomic_load(&identity_busy)&&identity_ready()&&setup_cookie(r);
    xSemaphoreGive(lock);
    if(!allowed)return http_error(r,"403 Forbidden","Physical pairing required");
    if(r->method==HTTP_POST){
        cJSON *body=read_json(r);const char *keys[]={"forget"};const cJSON *name=field(body,"forget");
        if(!body||!object_keys(body,keys,1)||!cJSON_IsString(name)||!name->valuestring[0]){cJSON_Delete(body);return http_error(r,"400 Bad Request","Network name required");}
        esp_err_t result=wireless_forget(name->valuestring);cJSON_Delete(body);
        if(result!=ESP_OK)return http_error(r,"409 Conflict","Network could not be removed");
    }
    char names[5][33];unsigned count=wireless_saved_names(names);
    cJSON *reply=cJSON_CreateObject();cJSON *list=cJSON_AddArrayToObject(reply,"networks");
    for(unsigned i=0;i<count;i++){cJSON *item=cJSON_CreateObject();cJSON_AddStringToObject(item,"ssid",names[i]);cJSON_AddItemToArray(list,item);}
    cJSON_AddNumberToObject(reply,"limit",5);return send_json(r,"200 OK",reply);
}
static esp_err_t wifi_config_handler(httpd_req_t *r) {
    const char *scope_error=setup_transport_error(r);
    if (scope_error) return http_error(r,"403 Forbidden",scope_error);
    cJSON *body=read_json(r);
    const char *keys[]={"ssid","password"};
    const cJSON *ssid=field(body,"ssid"),*password=field(body,"password");
    if (!body || !object_keys(body,keys,2) || !cJSON_IsString(ssid) || !cJSON_IsString(password)) {
        cJSON_Delete(body); return http_error(r,"400 Bad Request","SSID and password required");
    }
    xSemaphoreTake(lock,portMAX_DELAY);
    bool allowed=setup_valid_locked() && pair_state==PAIR_READY &&
                 !atomic_load(&identity_busy) && identity_ready() && setup_cookie(r);
    xSemaphoreGive(lock);
    esp_err_t result=allowed ? wireless_configure(ssid->valuestring,password->valuestring) : ESP_ERR_INVALID_STATE;
    mbedtls_platform_zeroize(password->valuestring,strlen(password->valuestring));
    cJSON_Delete(body);
    if (!allowed) return http_error(r,"403 Forbidden","Confirm physical MCP setup on the device first");
    if(result==ESP_ERR_NO_MEM)return http_error(r,"409 Conflict","Up to five networks can be saved; remove a saved network first");
    if (result!=ESP_OK) return http_error(r,"400 Bad Request","Wi-Fi settings could not be applied; check SSID/password and device status");
    cJSON *reply=cJSON_CreateObject(); cJSON_AddBoolToObject(reply,"saved",true);
    return send_json(r,"200 OK",reply);
}
static esp_err_t wifi_scan_handler(httpd_req_t *r) {
    const char *scope_error=setup_transport_error(r);
    if (scope_error) return http_error(r,"403 Forbidden",scope_error);
    xSemaphoreTake(lock,portMAX_DELAY);
    bool allowed=setup_valid_locked() && pair_state==PAIR_READY &&
                 !atomic_load(&identity_busy) && identity_ready() && setup_cookie(r);
    xSemaphoreGive(lock);
    if (!allowed) return http_error(r,"403 Forbidden","Confirm physical MCP setup on the device first");
    if (r->method==HTTP_POST) {
        cJSON *body=read_json(r);
        bool valid=body && empty_object(body); cJSON_Delete(body);
        if (!valid) return http_error(r,"400 Bad Request","Empty JSON object required");
        esp_err_t result=wireless_scan_begin();
        if (result!=ESP_OK) return http_error(r,"409 Conflict","Wi-Fi busy or unavailable; try again after configuration finishes");
        cJSON *reply=cJSON_CreateObject(); cJSON_AddStringToObject(reply,"state","scanning");
        return send_json(r,"200 OK",reply);
    }
    wireless_scan_result scan; wireless_scan_snapshot(&scan);
    cJSON *reply=cJSON_CreateObject();
    cJSON_AddStringToObject(reply,"state",scan.busy ? "scanning" : !scan.done ? "idle" : scan.error ? "failed" : "ready");
    cJSON_AddNumberToObject(reply,"error",scan.error);
    cJSON *networks=cJSON_AddArrayToObject(reply,"networks");
    if (!scan.busy && scan.done && !scan.error) for (unsigned i=0;i<scan.count;++i) {
        const wireless_network *network=&scan.networks[i];
        cJSON *item=cJSON_CreateObject();
        cJSON_AddStringToObject(item,"ssid",network->ssid);
        cJSON_AddNumberToObject(item,"rssi",network->rssi);
        cJSON_AddBoolToObject(item,"open",network->open);
        cJSON_AddItemToArray(networks,item);
    }
    return send_json(r,"200 OK",reply);
}
static esp_err_t profile_handler(httpd_req_t *r) {
    const char *scope_error = usb_scope_error(r, false);
    if (scope_error)
        return http_error(r, "403 Forbidden", scope_error);
    xSemaphoreTake(lock, portMAX_DELAY);
    if (!setup_valid_locked() || pair_state != PAIR_READY || atomic_load(&identity_busy) ||
        !identity_ready() || !setup_cookie(r)) {
        xSemaphoreGive(lock);
        return http_error(r, "403 Forbidden", "Physically approved setup required");
    }
    size_t len = 0, encoded = 0;
    const unsigned char *der = identity_ca_der(&len);
    unsigned char base64[1500];
    int result = mbedtls_base64_encode(base64, sizeof(base64) - 1, &encoded, der, len);
    base64[encoded] = 0;
    char uuid[40];
    const char *fp = identity_fingerprint();
    snprintf(uuid, sizeof(uuid), "%.8s-%.4s-%.4s-%.4s-%.12s", fp, fp + 8, fp + 12, fp + 16,
             fp + 20);
    char *profile = malloc(3500);
    if (result || !profile) {
        xSemaphoreGive(lock);
        free(profile);
        return http_error(r, "503 Service Unavailable", "Profile unavailable");
    }
    int size = snprintf(
        profile, 3500,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST "
        "1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"><plist version=\"1.0\"><dict>"
        "<key>PayloadType</key><string>Configuration</string><key>PayloadVersion</key><integer>1</"
        "integer><key>PayloadIdentifier</key><string>local.stopwatch.%s</string>"
        "<key>PayloadUUID</key><string>%s</string><key>PayloadDisplayName</key><string>StopWatch "
        "Local CA</string><key>PayloadDescription</key><string>Trust only your physically paired "
        "StopWatch. Enable full certificate trust in Settings after installation.</string>"
        "<key>PayloadContent</key><array><dict><key>PayloadType</"
        "key><string>com.apple.security.root</string><key>PayloadVersion</key><integer>1</integer>"
        "<key>PayloadIdentifier</key><string>local.stopwatch.root.%s</string><key>PayloadUUID</"
        "key><string>%.8s-%.4s-%.4s-%.4s-%.12s</string>"
        "<key>PayloadDisplayName</key><string>StopWatch Local "
        "CA</string><key>PayloadCertificateFileName</key><string>stopwatch-ca.cer</"
        "string><key>PayloadContent</key><data>%s</data></dict></array></dict></plist>",
        uuid, uuid, uuid, fp + 32, fp + 40, fp + 44, fp + 48, fp + 52, base64);
    xSemaphoreGive(lock);
    if (size < 0 || size >= 3500) {
        free(profile);
        return ESP_FAIL;
    }
    response_headers(r);
    httpd_resp_set_type(r, "application/x-apple-aspen-config");
    httpd_resp_set_hdr(r, "Content-Disposition", "attachment; filename=stopwatch.mobileconfig");
    esp_err_t sent = httpd_resp_send(r, profile, size);
    free(profile);
    return sent;
}
static bool register_uri(httpd_handle_t server, const char *uri, httpd_method_t method,
                         esp_err_t (*handler)(httpd_req_t *)) {
    httpd_uri_t route = {.uri = uri,
                         .method = method,
                         .handler = handler,
                         .user_ctx = server == https_server ? (void *)&secure_route : NULL};
    esp_err_t result = httpd_register_uri_handler(server, &route);
    if (result != ESP_OK && server == http_server)
        atomic_store(&http_start_error, result);
    return result == ESP_OK;
}
// Public build-time project source only; no identity/configuration reads.
static esp_err_t source_handler(httpd_req_t *r) {
    extern const unsigned char source_start[] asm("_binary_stopwatch_source_start");
    extern const unsigned char source_end[] asm("_binary_stopwatch_source_end");
    response_headers(r);
    httpd_resp_set_type(r,"application/gzip");
    httpd_resp_set_hdr(r,"Content-Disposition","attachment; filename=stopwatch-source.tar.gz");
    for(const unsigned char *p=source_start;p<source_end;){
        size_t n=(size_t)(source_end-p);if(n>2048)n=2048;
        esp_err_t result=httpd_resp_send_chunk(r,(const char *)p,n);
        if(result!=ESP_OK)return result;
        p+=n;
    }
    return httpd_resp_send_chunk(r,NULL,0);
}
static bool start_https(void) {
    if (!identity_ready())
        return false;
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.port_secure = 8443;
    config.httpd.ctrl_port = 32769;
    config.httpd.max_open_sockets = 4;
    config.httpd.max_uri_handlers = 7;
    config.httpd.stack_size = 8192;
    config.httpd.recv_wait_timeout = 2;
    config.httpd.send_wait_timeout = 2;
    config.httpd.lru_purge_enable = true;
    config.servercert = (const unsigned char *)identity_certificate();
    config.servercert_len = strlen(identity_certificate()) + 1;
    config.prvtkey_pem = (const unsigned char *)identity_private_key();
    config.prvtkey_len = strlen(identity_private_key()) + 1;
    if (httpd_ssl_start(&https_server, &config) != ESP_OK)
        return false;
    bool ok = register_uri(https_server, "/mcp", HTTP_POST, mcp_handler) &&
              register_uri(https_server, "/mcp", HTTP_GET, mcp_handler) &&
              register_uri(https_server, "/mcp", HTTP_DELETE, mcp_handler) &&
              register_uri(https_server, "/setup", HTTP_GET, page_handler) &&
              register_uri(https_server, "/config", HTTP_POST, config_handler) &&
              register_uri(https_server, "/source", HTTP_GET, source_handler);
    if (!ok) {
        httpd_ssl_stop(https_server);
        https_server = NULL;
    }
    atomic_store(&https_running, ok);
    return ok;
}
static void identity_worker(void *unused) {
    (void)unused;
    identity_job job;
    for (;;)
        if (xQueueReceive(identity_jobs, &job, portMAX_DELAY) == pdTRUE) {
            // HTTP stays up during maintenance: serialize secret readers before mutation.
            xSemaphoreTake(lock, portMAX_DELAY);
            atomic_store(&identity_busy, true);
            xSemaphoreGive(lock);
            device_stop();
            // HTTP async ownership must be returned before stopping the TLS server.
            for (;;) {
                xSemaphoreTake(lock, portMAX_DELAY);
                bool busy = active != NULL;
                xSemaphoreGive(lock);
                if (!busy)
                    break;
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            atomic_store(&https_running, false);
            if (https_server) {
                httpd_ssl_stop(https_server);
                https_server = NULL;
            }
            xSemaphoreTake(lock, portMAX_DELAY);
            memset(sessions, 0, sizeof(sessions));
            owner[0] = 0;
            bool valid = job.reset || (job.setup_generation == setup_generation &&
                                       setup_valid_locked() && pair_state == PAIR_WORKING);
            xSemaphoreGive(lock);
            bool ok = false;
            if (valid)
                ok = (job.reset ? identity_reset() : identity_prepare(job.now)) == ESP_OK;
            if (identity_ready())
                start_https();
            xSemaphoreTake(lock, portMAX_DELAY);
            atomic_store(&identity_busy, false);
            if (job.reset) {
                ++setup_generation;
                setup_until = now_ms() + SETUP_TTL;
                pair_state = PAIR_NONE;
                setup_nonce[0] = 0;
            } else if (job.setup_generation == setup_generation)
                pair_state = ok && mcp_ready() ? PAIR_READY : PAIR_FAILED;
            xSemaphoreGive(lock);
            device_setup_finished(ok);
        }
}
void mcp_init(void) {
    lock = xSemaphoreCreateMutex();
    identity_jobs = xQueueCreate(1, sizeof(identity_job));
    configASSERT(lock && identity_jobs);
    identity_init();
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32768;
    config.stack_size = 8192;
    config.max_open_sockets = 4;
    config.max_uri_handlers = 15;
    config.lru_purge_enable = true;
    config.recv_wait_timeout = 2;
    config.send_wait_timeout = 2;
    esp_err_t result = httpd_start(&http_server, &config);
    atomic_store(&http_start_error, result);
    if (result == ESP_OK) {
        bool ok =
            register_uri(http_server, "/", HTTP_GET, page_handler) &&
            register_uri(http_server, "/status", HTTP_GET, diagnostic_handler) &&
            register_uri(http_server, "/enroll/start", HTTP_POST, enrollment_handler) &&
            register_uri(http_server, "/enroll/status", HTTP_POST, enrollment_handler) &&
            register_uri(http_server, "/certificate.mobileconfig", HTTP_GET, profile_handler) &&
            register_uri(http_server, "/config", HTTP_POST, config_handler) &&
            register_uri(http_server, "/wifi", HTTP_POST, wifi_config_handler) &&
            register_uri(http_server, "/wifi/networks", HTTP_GET, wifi_networks_handler) &&
            register_uri(http_server, "/wifi/networks", HTTP_POST, wifi_networks_handler) &&
            register_uri(http_server, "/wifi/scan", HTTP_POST, wifi_scan_handler) &&
            register_uri(http_server, "/wifi/scan", HTTP_GET, wifi_scan_handler) &&
            register_uri(http_server, "/mcp", HTTP_POST, mcp_handler) &&
            register_uri(http_server, "/mcp", HTTP_GET, mcp_handler) &&
            register_uri(http_server, "/mcp", HTTP_DELETE, mcp_handler) &&
            register_uri(http_server, "/source", HTTP_GET, source_handler);
        if (!ok) {
            httpd_stop(http_server);
            http_server = NULL;
        }
        atomic_store(&http_running, ok);
    }
    if (identity_ready())
        start_https();
    configASSERT(xTaskCreate(identity_worker, "mcp-identity", 8192, NULL, 2, NULL) == pdPASS);
}

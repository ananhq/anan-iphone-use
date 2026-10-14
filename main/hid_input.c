// SPDX-License-Identifier: Apache-2.0
// First-party bounded parsing. Usage values follow pinned TinyUSB/Apple docs.
#include "hid_input.h"
#include <math.h>
#include <string.h>

static const cJSON *get(const cJSON *o, const char *key) {
    return cJSON_GetObjectItemCaseSensitive(o, key);
}
static bool keys_only(const cJSON *o, const char *const *keys, size_t count) {
    if (!cJSON_IsObject(o)) return false;
    for (const cJSON *p = o->child; p; p = p->next) {
        bool found = false;
        for (size_t i = 0; i < count; ++i)
            if (!strcmp(p->string, keys[i])) found = true;
        if (!found) return false;
        for (const cJSON *q = p->next; q; q = q->next)
            if (!strcmp(p->string, q->string)) return false;
    }
    return true;
}
static bool number(const cJSON *v, double min, double max) {
    return cJSON_IsNumber(v) && isfinite(v->valuedouble) &&
           v->valuedouble >= min && v->valuedouble <= max;
}
static bool whole(const cJSON *v, int min, int max) {
    return number(v, min, max) && floor(v->valuedouble) == v->valuedouble;
}
static bool optional(const cJSON *o, const char *name, int min, int max, int fallback, int *value) {
    const cJSON *v = get(o, name);
    if (!v) { *value = fallback; return true; }
    if (!whole(v, min, max)) return false;
    *value = (int)v->valuedouble;
    return true;
}
static bool point(const cJSON *o, const char *name, uint16_t *value) {
    const cJSON *v = get(o, name);
    if (!number(v, 0, 1)) return false;
    *value = (uint16_t)lround(v->valuedouble * 65535.0);
    return true;
}
typedef struct { const char *name; uint8_t key, modifiers; bool repeatable; } named_key;
static const named_key keyboard_keys[] = {
    {"tab",43,0,true}, {"shift_tab",43,2,true},
    {"enter",40,0,false}, {"escape",41,0,false}, {"space",44,0,false},
    {"backspace",42,0,false}, {"delete",76,0,false},
    {"up",82,0,true}, {"down",81,0,true}, {"left",80,0,true}, {"right",79,0,true},
    {"command_space",44,8,false}, {"select_all",4,8,false},
    {"copy",6,8,false}, {"paste",25,8,false}, {"cut",27,8,false},
    {"undo",29,8,false}, {"redo",29,10,false},
    {"select_left",80,2,true}, {"select_right",79,2,true},
    {"word_left",80,4,true}, {"word_right",79,4,true}
};
typedef struct { const char *name; uint16_t usage; } system_key;
// Apple Accessory Design Guidelines 2026-09-21 table 14-3; usages, not Apple IDs.
static const system_key system_keys[] = {
    {"home",0x0040}, {"search",0x0221}, {"toggle_keyboard",0x01ae},
    {"globe",0x029d}, {"volume_up",0x00e9}, {"volume_down",0x00ea},
    {"mute",0x00e2}, {"play_pause",0x00cd},
    {"next_track",0x00b5}, {"previous_track",0x00b6},
    {"brightness_up",0x006f}, {"brightness_down",0x0070}
};
bool hid_input_is_tool(const char *name) {
    static const char *const names[] = {
        "tap", "drag", "press_key", "keyboard", "scroll", "long_press", "double_tap", "drag_hold", "draw_path", "system_key"
    };
    for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
        if (!strcmp(name, names[i])) return true;
    return false;
}
bool hid_input_parse(const char *name, const cJSON *args, action *out) {
    memset(out, 0, sizeof(*out));
    int repeat, press, gap, duration, hold, horizontal;
    if (!strcmp(name, "tap")) {
        const char *const keys[] = {"x", "y", "count", "holdMs", "intervalMs"};
        int count;
        if (!keys_only(args,keys,5) || !point(args,"x",&out->x) || !point(args,"y",&out->y) ||
            !optional(args,"count",1,2,1,&count) || !optional(args,"holdMs",0,2000,0,&hold) ||
            !optional(args,"intervalMs",50,300,100,&gap) || (hold && hold < 100) ||
            (count == 2 && hold) || (count == 1 && get(args,"intervalMs"))) return false;
        out->kind = count == 2 ? ACTION_DOUBLE_TAP : hold ? ACTION_LONG_PRESS : ACTION_TAP;
        out->duration = hold; out->gap_ms = gap;
        return true;
    }
    if (!strcmp(name, "drag") && !get(args,"points")) {
        const char *const keys[] = {"x", "y", "toX", "toY", "durationMs"};
        if (!keys_only(args,keys,5) || !point(args,"x",&out->x) || !point(args,"y",&out->y) ||
            !point(args,"toX",&out->end_x) || !point(args,"toY",&out->end_y) ||
            !whole(get(args,"durationMs"),100,2000)) return false;
        out->kind = ACTION_DRAG; out->duration = get(args,"durationMs")->valuedouble;
        return true;
    }
    if (!strcmp(name, "keyboard") || !strcmp(name, "press_key")) {
        const char *const keys[] = {"key", "repeat", "pressMs", "gapMs"};
        const cJSON *key = get(args, "key");
        if (!keys_only(args, keys, 4) || !cJSON_IsString(key) ||
            !optional(args,"repeat",1,5,1,&repeat) ||
            !optional(args,"pressMs",20,100,40,&press) ||
            !optional(args,"gapMs",50,300,100,&gap)) return false;
        for (unsigned i = 0; i < sizeof(keyboard_keys) / sizeof(keyboard_keys[0]); ++i) {
            const named_key *k = &keyboard_keys[i];
            if (strcmp(key->valuestring, k->name)) continue;
            if (repeat > 1 && !k->repeatable) return false;
            out->kind = ACTION_KEYBOARD; out->key = k->key; out->modifiers = k->modifiers;
            out->repeat = repeat; out->press_ms = press; out->gap_ms = gap;
            return true;
        }
        if (!strcmp(name,"press_key") && repeat == 1) {
            for (unsigned i = 0; i < sizeof(system_keys) / sizeof(system_keys[0]); ++i) {
                if (strcmp(key->valuestring,system_keys[i].name)) continue;
                out->kind = ACTION_CONSUMER; out->consumer = system_keys[i].usage;
                out->press_ms = press;
                return true;
            }
        }
        return false;
    }
    if (!strcmp(name, "system_key")) {
        const char *const keys[] = {"action"};
        const cJSON *key = get(args, "action");
        if (!keys_only(args, keys, 1) || !cJSON_IsString(key)) return false;
        for (unsigned i = 0; i < sizeof(system_keys) / sizeof(system_keys[0]); ++i) {
            if (strcmp(key->valuestring, system_keys[i].name)) continue;
            out->kind = ACTION_CONSUMER; out->consumer = system_keys[i].usage;
            out->press_ms = 40;
            return true;
        }
        return false;
    }
    if (!strcmp(name, "draw_path") || !strcmp(name, "drag")) {
        const char *const keys[] = {"points", "durationMs", "holdMs"};
        const cJSON *points = get(args, "points"), *d = get(args, "durationMs");
        int count = cJSON_GetArraySize(points);
        if (!keys_only(args, keys, 3) || !cJSON_IsArray(points) || count < 2 || count > ACTION_PATH_LIMIT ||
            !whole(d,100,2000) || d->valuedouble < (count - 1) * 20 ||
            !optional(args,"holdMs",0,1000,0,&hold)) return false;
        const char *const xy[] = {"x", "y"};
        for (int i = 0; i < count; ++i) {
            const cJSON *p = cJSON_GetArrayItem(points, i);
            if (!keys_only(p,xy,2) || !point(p,"x",&out->points[i].x) ||
                !point(p,"y",&out->points[i].y)) return false;
        }
        out->kind = ACTION_PATH; out->point_count = count;
        out->duration = d->valuedouble; out->hold_ms = hold;
        out->x = out->points[0].x; out->y = out->points[0].y;
        // Preserve the original straight-drag pacing for an unheld two-point drag.
        if (!strcmp(name,"drag") && count == 2) {
            out->kind = hold ? ACTION_DRAG_HOLD : ACTION_DRAG;
            out->end_x = out->points[1].x; out->end_y = out->points[1].y;
        }
        return true;
    }
    if (!strcmp(name, "scroll")) {
        const char *const keys[] = {"x", "y", "vertical", "horizontal", "repeat", "gapMs"};
        const cJSON *v = get(args,"vertical");
        if (!keys_only(args,keys,6) || !point(args,"x",&out->x) || !point(args,"y",&out->y) ||
            !whole(v,-10,10) || !optional(args,"horizontal",-10,10,0,&horizontal) ||
            !optional(args,"repeat",1,5,1,&repeat) || !optional(args,"gapMs",50,300,100,&gap) ||
            (v->valuedouble == 0 && horizontal == 0)) return false;
        out->kind = ACTION_SCROLL; out->vertical = v->valuedouble; out->horizontal = horizontal;
        out->repeat = repeat; out->gap_ms = gap;
        return true;
    }
    if (!strcmp(name,"long_press")) {
        const char *const keys[] = {"x", "y", "durationMs"};
        if (!keys_only(args,keys,3) || !point(args,"x",&out->x) || !point(args,"y",&out->y) ||
            !whole(get(args,"durationMs"),100,2000)) return false;
        out->kind = ACTION_LONG_PRESS; out->duration = get(args,"durationMs")->valuedouble;
        return true;
    }
    if (!strcmp(name,"double_tap")) {
        const char *const keys[] = {"x", "y", "intervalMs"};
        if (!keys_only(args,keys,3) || !point(args,"x",&out->x) || !point(args,"y",&out->y) ||
            !optional(args,"intervalMs",50,300,100,&gap)) return false;
        out->kind = ACTION_DOUBLE_TAP; out->gap_ms = gap;
        return true;
    }
    if (!strcmp(name,"drag_hold")) {
        const char *const keys[] = {"x", "y", "toX", "toY", "durationMs", "holdMs"};
        if (!keys_only(args,keys,6) || !point(args,"x",&out->x) || !point(args,"y",&out->y) ||
            !point(args,"toX",&out->end_x) || !point(args,"toY",&out->end_y) ||
            !whole(get(args,"durationMs"),100,2000) ||
            !optional(args,"holdMs",100,1000,500,&hold)) return false;
        duration = get(args,"durationMs")->valuedouble;
        out->kind = ACTION_DRAG_HOLD; out->duration = duration; out->hold_ms = hold;
        return true;
    }
    return false;
}
const char *hid_input_unit(action_kind kind) {
    switch (kind) {
    case ACTION_KEYBOARD: return "key_presses";
    case ACTION_SCROLL: return "wheel_reports";
    case ACTION_DOUBLE_TAP: return "clicks";
    case ACTION_PATH: return "path_segments";
    case ACTION_CONSUMER: return "consumer_presses";
    default: return "gestures";
    }
}
unsigned hid_input_total(const action *a) {
    if (a->kind == ACTION_KEYBOARD || a->kind == ACTION_SCROLL) return a->repeat;
    if (a->kind == ACTION_DOUBLE_TAP) return 2;
    if (a->kind == ACTION_PATH) return a->point_count - 1;
    return 1;
}

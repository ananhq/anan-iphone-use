// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum { ACTION_TAP, ACTION_DRAG, ACTION_KEY, ACTION_TYPE } action_kind;
typedef struct {
    uint32_t epoch; // Worker guard token; MCP owns request/device generations in its context.
    action_kind kind;
    uint16_t x, y, end_x, end_y, duration;
    uint8_t key, modifiers, text[120], text_length;
} action;
typedef bool (*action_guard)(uint32_t owner, void *context);
typedef void (*action_done)(const action *a, bool succeeded, void *context);
void actions_init(void);
bool actions_busy(void);
bool actions_reserve(void);
void actions_unreserve(void);
bool actions_enqueue(const action *a, action_guard guard, action_done done, void *context);
void actions_request_release(void);
bool ascii_key(uint8_t c, uint8_t *key, uint8_t *modifiers);

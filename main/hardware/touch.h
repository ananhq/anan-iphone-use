// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    bool pressed;
    uint16_t x, y;
    uint8_t event;
} touch_point;
bool touch_init(void);
bool touch_read(touch_point *point);
bool touch_ready(void);
int touch_error(void);

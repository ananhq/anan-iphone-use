// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool display_init(void);
void display_text(const char *title, const char *body, const char *footer);
bool display_ready(void);
const char *display_status(void);
int display_error(void);
bool display_reset_touch(void);
uint8_t display_ascii_row(char character, unsigned row);
typedef void (*display_strip_painter)(uint16_t *pixels, int strip_y, void *context);
bool display_paint(display_strip_painter painter, void *context);

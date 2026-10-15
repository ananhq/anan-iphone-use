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
bool display_paint_region(int x,int y,int end,int bottom,display_strip_painter painter,void *context);
typedef struct {int x,y,w,h;} display_region;
typedef struct {uint32_t transfers,pixels;} display_paint_metrics;
typedef void (*display_region_painter)(uint16_t *pixels,int x,int y,int width,int rows,void *context);
bool display_paint_regions(const display_region *regions,unsigned count,display_region_painter painter,void *context);
display_paint_metrics display_last_paint_metrics(void);

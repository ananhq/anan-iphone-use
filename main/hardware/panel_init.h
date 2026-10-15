/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 M5Stack Technology CO LTD
 * Panel command data adapted from main/hal/hal_display.cpp in
 * M5StopWatch-UserDemo 6b4aa125288b6fe9dca661f10159f6e1e5ee785c.
 * Complete notice: ../licenses/M5Stack-demo-MIT.txt.
 */
#pragma once
#include "esp_lcd_co5300.h"
static const uint8_t panel_c4[] = {0x80}, panel_te[] = {0x80}, panel_line[] = {0x01, 0xd2};
static const uint8_t panel_ctrl[] = {0x20}, panel_direction[] = {0x00}, panel_brightness[] = {0xa0};
static const co5300_lcd_init_cmd_t stopwatch_panel_commands[] = {
    {0x11, NULL, 0, 150},          {0xc4, panel_c4, 1, 0},         {0x35, panel_te, 1, 0},
    {0x44, panel_line, 2, 0},      {0x53, panel_ctrl, 1, 0},       {0x20, NULL, 0, 0},
    {0x36, panel_direction, 1, 0}, {0x51, panel_brightness, 1, 0}, {0x29, NULL, 0, 0}};

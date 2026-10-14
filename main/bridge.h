// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>

void board_init(void);
bool board_stop_pressed(void);
void bridge_usb_init(void);
bool bridge_usb_online(void);
bool bridge_mouse(uint8_t buttons, uint16_t x, uint16_t y);
bool bridge_scroll(uint16_t x, uint16_t y, int8_t vertical, int8_t horizontal);
bool bridge_key(uint8_t modifiers, uint8_t key);
bool bridge_consumer(uint16_t usage);
void bridge_release(void);

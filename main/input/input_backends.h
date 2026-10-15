// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
bool usb_mouse(uint8_t buttons, uint16_t x, uint16_t y);
bool usb_scroll(uint16_t x, uint16_t y, int8_t vertical, int8_t horizontal);
bool usb_key(uint8_t modifiers, uint8_t key);
bool usb_consumer(uint16_t usage);
void usb_release(void);
void ble_input_init(void);
void ble_input_enable(bool enabled);
bool ble_input_online(void);
bool ble_input_connected(void);
bool ble_input_ready(void);
int ble_input_error(void);
bool ble_input_forget(void);
bool ble_mouse(uint8_t buttons, uint16_t x, uint16_t y);
bool ble_scroll(uint16_t x, uint16_t y, int8_t vertical, int8_t horizontal);
bool ble_key(uint8_t modifiers, uint8_t key);
bool ble_consumer(uint16_t usage);
void ble_release(void);

// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdint.h>
void ble_pointer_payload(uint8_t out[7], uint16_t x, uint16_t y);
void ble_relative_payload(uint8_t out[5], uint8_t buttons, int8_t wheel, int8_t pan);
void ble_keyboard_payload(uint8_t out[8], uint8_t modifiers, uint8_t key);
void ble_consumer_payload(uint8_t out[2], uint16_t usage);

// SPDX-License-Identifier: Apache-2.0
#include "ble_reports.h"
#include <string.h>
void ble_pointer_payload(uint8_t out[7], uint8_t b, uint16_t x, uint16_t y) {
    x >>= 1; y >>= 1;
    const uint8_t p[7] = {b, x & 255, x >> 8, y & 255, y >> 8, 0, 0};
    memcpy(out, p, sizeof(p));
}
void ble_relative_payload(uint8_t out[5], uint8_t b, int8_t wheel, int8_t pan) {
    const uint8_t p[5] = {b, 0, 0, (uint8_t)wheel, (uint8_t)pan};
    memcpy(out, p, sizeof(p));
}
void ble_keyboard_payload(uint8_t out[8], uint8_t m, uint8_t k) {
    memset(out, 0, 8); out[0] = m; out[2] = k;
}
void ble_consumer_payload(uint8_t out[2], uint16_t u) { out[0] = u & 255; out[1] = u >> 8; }

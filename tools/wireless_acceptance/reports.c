// SPDX-License-Identifier: Apache-2.0
// Pre-implementation payload checks. No Bluetooth or phone operations.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ble_reports.h"
int main(void) {
    uint8_t p[7], r[5], k[8], c[2];
    ble_pointer_payload(p, 65535, 0);
    const uint8_t end[] = {0, 255, 127, 0, 0, 0, 0};
    assert(!memcmp(p, end, sizeof(end)));
    ble_pointer_payload(p, 0, 65535);
    const uint8_t other[] = {0, 0, 0, 255, 127, 0, 0};
    assert(!memcmp(p, other, sizeof(other)));
    ble_pointer_payload(p, 32768, 32768);
    assert(p[0] == 0 && p[1] == 0 && p[2] == 64 && p[4] == 64);
    ble_relative_payload(r, 1, -127, 127);
    const uint8_t wheel[] = {1, 0, 0, 129, 127};
    assert(!memcmp(r, wheel, sizeof(wheel)));
    ble_keyboard_payload(k, 8, 4);
    const uint8_t key[] = {8, 0, 4, 0, 0, 0, 0, 0};
    assert(!memcmp(k, key, sizeof(key)));
    ble_keyboard_payload(k, 0, 0);
    for (unsigned i = 0; i < sizeof(k); ++i) assert(k[i] == 0);
    ble_consumer_payload(c, 0x029d);
    assert(c[0] == 0x9d && c[1] == 2);
    ble_consumer_payload(c, 0);
    assert(c[0] == 0 && c[1] == 0);
    puts("BLE coordinate/button/keyboard/Consumer payload scenarios passed (offline).");
}

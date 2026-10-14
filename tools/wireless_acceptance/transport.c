// SPDX-License-Identifier: Apache-2.0
// Defined before implementation. Host simulation only: no radio or USB access.
#include <assert.h>
#include <stdio.h>
#include "bridge.h"
#include "input_backends.h"
static bool usb_up, ble_up, busy, ble_enabled;
static unsigned usb_reports, ble_reports, usb_releases, ble_releases;
bool actions_busy(void) { return busy; }
bool bridge_usb_online(void) { return usb_up; }
bool ble_input_online(void) { return ble_up; }
void ble_input_enable(bool enabled) { ble_enabled = enabled; }
bool usb_mouse(uint8_t b, uint16_t x, uint16_t y) { (void)b;(void)x;(void)y; ++usb_reports; return usb_up; }
bool ble_mouse(uint8_t b, uint16_t x, uint16_t y) { (void)b;(void)x;(void)y; ++ble_reports; return ble_up; }
bool usb_key(uint8_t m, uint8_t k) { (void)m;(void)k; return usb_up; }
bool ble_key(uint8_t m, uint8_t k) { (void)m;(void)k; return ble_up; }
bool usb_scroll(uint16_t x,uint16_t y,int8_t v,int8_t h) { (void)x;(void)y;(void)v;(void)h; return usb_up; }
bool ble_scroll(uint16_t x,uint16_t y,int8_t v,int8_t h) { (void)x;(void)y;(void)v;(void)h; return ble_up; }
bool usb_consumer(uint16_t u) { (void)u; return usb_up; }
bool ble_consumer(uint16_t u) { (void)u; return ble_up; }
void usb_release(void) { ++usb_releases; }
void ble_release(void) { ++ble_releases; }
int main(void) {
    assert(bridge_input_mode() == INPUT_USB);
    usb_up = true; ble_up = true;
    assert(bridge_input_online() && bridge_mouse(1, 65535, 0));
    assert(usb_reports == 1 && ble_reports == 0);
    busy = true;
    assert(!bridge_select_input(INPUT_BLE) && bridge_input_mode() == INPUT_USB);
    busy = false;
    assert(bridge_select_input(INPUT_BLE) && ble_enabled && usb_releases == 1);
    assert(bridge_mouse(1, 0, 65535) && ble_reports == 1);
    ble_up = false;
    assert(!bridge_input_online() && !bridge_mouse(1, 0, 0));
    assert(usb_reports == 1); // Never fall back to the still-connected USB host.
    assert(bridge_select_input(INPUT_USB) && !ble_enabled && ble_releases == 1);
    assert(!bridge_select_input((input_mode_t)99));
    bridge_release();
    assert(usb_releases == 2);
    puts("Explicit input selection/no-fallback scenarios passed (simulated).");
}

// SPDX-License-Identifier: Apache-2.0
#include "bridge.h"
#include "input_backends.h"
#include "actions.h"
#include "eyes.h"
#include <stdatomic.h>
static atomic_int selected = INPUT_USB;
#ifndef INPUT_HOST_CHECK
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static SemaphoreHandle_t report_mutex;
void bridge_input_init(void) { report_mutex=xSemaphoreCreateMutex(); configASSERT(report_mutex); }
static void take(void) { xSemaphoreTake(report_mutex,portMAX_DELAY); }
static void give(void) { xSemaphoreGive(report_mutex); }
#else
// The pre-defined offline fixture exercises routing only, not SDK mutex behavior.
void bridge_input_init(void) {}
static void take(void) {}
static void give(void) {}
#endif
input_mode_t bridge_input_mode(void) { return atomic_load(&selected); }
const char *bridge_input_name(void) { return bridge_input_mode() == INPUT_BLE ? "ble" : "usb"; }
bool bridge_input_online(void) {
    return bridge_input_mode() == INPUT_BLE ? ble_input_online() : bridge_usb_online();
}
bool bridge_select_input(input_mode_t mode) {
    if ((mode != INPUT_USB && mode != INPUT_BLE) || actions_busy()) return false;
    take();
    if (actions_busy()) { give(); return false; }
    if (mode == bridge_input_mode()) { give(); return true; }
    if (bridge_input_mode()==INPUT_BLE) ble_release(); else usb_release();
    atomic_store(&selected, mode);
    ble_input_enable(mode == INPUT_BLE);
    give();
    return true;
}
bool bridge_mouse(uint8_t b, uint16_t x, uint16_t y) {
    take(); bool ok=bridge_input_mode() == INPUT_BLE ? ble_mouse(b,x,y) : usb_mouse(b,x,y); give();
    if(ok)eyes_pointer(x,y);
    return ok;
}
bool bridge_scroll(uint16_t x,uint16_t y,int8_t v,int8_t h) {
    take(); bool ok=bridge_input_mode() == INPUT_BLE ? ble_scroll(x,y,v,h) : usb_scroll(x,y,v,h); give(); return ok;
}
bool bridge_key(uint8_t m,uint8_t k) {
    take(); bool ok=bridge_input_mode() == INPUT_BLE ? ble_key(m,k) : usb_key(m,k); give(); return ok;
}
bool bridge_consumer(uint16_t u) {
    take(); bool ok=bridge_input_mode() == INPUT_BLE ? ble_consumer(u) : usb_consumer(u); give(); return ok;
}
void bridge_release(void) {
    take();
    if (bridge_input_mode() == INPUT_BLE) ble_release(); else usb_release();
    give();
}

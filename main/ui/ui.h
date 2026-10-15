// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "device.h"
#include "bridge.h"
#include "eyes.h"
#include "avatar.h"
#include "reset_trace.h"
typedef enum { UI_ZH, UI_EN } ui_language;
typedef enum { UI_HOME, UI_SETTINGS, UI_PAIRING, UI_RESET, UI_RESETTING, UI_INPUT,
               UI_CONNECT, UI_MORE, UI_LANGUAGE, UI_BLE_RESET } ui_page;
typedef enum {
    UI_PAIR_IDLE,
    UI_PAIR_PENDING,
    UI_PAIR_WORKING,
    UI_PAIR_READY,
    UI_PAIR_FAILED,
    UI_PAIR_EXPIRED
} ui_pair_state;
typedef enum {
    UI_NONE,
    UI_START,
    UI_STOP,
    UI_SETTINGS_OPEN,
    UI_SETUP_OPEN,
    UI_BACK,
    UI_CONFIRM_PAIR,
    UI_SETUP_DONE,
    UI_RESET_OPEN,
    UI_RESET_HOLD,
    UI_ZH_SELECT,
    UI_EN_SELECT,
    UI_INPUT_OPEN,
    UI_USB_SELECT,
    UI_BLE_SELECT,
    UI_BLE_RESET_HOLD,
    UI_WIFI_SETUP_OPEN,
    UI_CONNECT_OPEN,
    UI_MORE_OPEN,
    UI_LANGUAGE_OPEN,
    UI_BLE_RESET_OPEN
} ui_action;
typedef struct {
    ui_page page;
    ui_language language;
    device_mode_t mode;
    ui_pair_state pairing;
    bool usb_online, identity_ready, mcp_ready, touch_ready, input_busy;
    bool control_granted;
    input_mode_t input_mode;
    bool input_online, ble_connected, ble_ready, wifi_configured, wifi_connected;
    char wifi_address[16];
    bool setup_ap;
    bool wifi_setup, http_ready;
    int wifi_setup_stage, wifi_setup_error, http_error;
    uint32_t free_internal_heap, minimum_internal_heap, largest_internal_block, free_psram;
    char firmware_version[32];
    int reset_reason;
    uint32_t uptime_seconds;
    reset_trace_info previous_ap;
    char ap_name[33], ap_password[13];
    uint32_t revision;
    ui_action focus;
    ui_action pressed;
    unsigned hold_percent;
    int battery_percent; // -1: unavailable; coarse voltage estimate, Home only.
    char code[7], fingerprint[17];
    eyes_pose eyes; // Visual only: never part of grant/gesture authorization.
    avatar_pose avatar;
    bool avatar_valid;
} ui_model;
typedef struct {
    int x, y, w, h;
} ui_rect;
typedef struct {
    ui_rect rect;
    char text[96];
    uint16_t foreground, background;
    unsigned scale;
    ui_action action;
    bool button, enabled, selected, qr;
} ui_widget;
#define UI_WIDGET_LIMIT 20
size_t ui_layout(const ui_model *model, ui_widget widgets[UI_WIDGET_LIMIT]);
ui_action ui_hit(const ui_model *model, unsigned x, unsigned y);
bool ui_target_contains(const ui_model *model, ui_action target, unsigned x, unsigned y,
                        unsigned margin);
ui_page ui_parent(ui_page page);
ui_action ui_next_focus(const ui_model *model);
void ui_init(void);
bool ui_present(const ui_model *model);
bool ui_painted_last_frame(void); // screen-task-only timing helper
bool ui_displayed(ui_model *model);

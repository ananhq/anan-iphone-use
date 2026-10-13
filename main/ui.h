// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "device.h"
typedef enum { UI_ZH, UI_EN } ui_language;
typedef enum { UI_HOME, UI_SETTINGS, UI_PAIRING, UI_RESET, UI_RESETTING } ui_page;
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
    UI_EN_SELECT
} ui_action;
typedef struct {
    ui_page page;
    ui_language language;
    device_mode_t mode;
    ui_pair_state pairing;
    bool usb_online, identity_ready, mcp_ready, touch_ready, input_busy;
    bool control_granted;
    uint32_t revision;
    ui_action focus;
    char code[7], fingerprint[17];
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
    bool button, enabled, selected;
} ui_widget;
#define UI_WIDGET_LIMIT 20
size_t ui_layout(const ui_model *model, ui_widget widgets[UI_WIDGET_LIMIT]);
ui_action ui_hit(const ui_model *model, unsigned x, unsigned y);
ui_action ui_next_focus(const ui_model *model);
void ui_init(void);
bool ui_present(const ui_model *model);
bool ui_displayed(ui_model *model);

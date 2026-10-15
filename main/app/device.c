// SPDX-License-Identifier: Apache-2.0
#include "device.h"
#include "actions.h"
#include "bridge.h"
#include "display.h"
#include "identity.h"
#include "mcp.h"
#include "touch.h"
#include "ui.h"
#include "ui_preferences.h"
#include "wireless.h"
#include "input_backends.h"
#include "battery.h"
#include "imu.h"
#include "avatar_timing.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_app_desc.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdatomic.h>
#include <string.h>
// Hit testing builds the shared layout and calls libc formatting. Each 4 KiB
// input-task stack was smaller than that target-compiled call chain.
#define UI_INPUT_STACK_BYTES 8192
static atomic_int mode, page, language, focus, setup_result;
static atomic_uint generation, ui_revision;
static atomic_bool screen_ok, control_granted;
static atomic_bool touch_pressed;
static atomic_bool wifi_setup;
static atomic_int battery_percent = ATOMIC_VAR_INIT(-1), setup_parent;
static atomic_uint touch_feedback, feedback_revision;
static portMUX_TYPE permission_lock = portMUX_INITIALIZER_UNLOCKED;
static uint32_t now_ms(void) { return esp_timer_get_time() / 1000; }
const char *device_ui_language(void) { return atomic_load(&language) == UI_EN ? "en" : "zh"; }
device_mode_t device_mode(void) { return atomic_load(&mode); }
uint32_t device_generation(void) { return atomic_load(&generation); }
bool device_control_granted(void) { return atomic_load(&control_granted); }
bool device_control_allowed(void) {
    return device_mode() == DEVICE_MCP && atomic_load(&screen_ok) && device_control_granted() &&
           bridge_input_online() && !wireless_snapshot().setup_ap && !board_stop_pressed();
}
void device_stop(void) {
    portENTER_CRITICAL(&permission_lock);
    atomic_store(&control_granted, false);
    atomic_fetch_add(&generation, 1);
    atomic_fetch_add(&ui_revision, 1);
    portEXIT_CRITICAL(&permission_lock);
    mcp_stop_input();
    actions_request_release();
}
void device_setup_finished(bool ok) {
    atomic_store(&setup_result, ok ? 1 : -1);
    atomic_fetch_add(&ui_revision, 1);
}
static void set_page(ui_page next) {
    atomic_store(&touch_feedback, 0);
    atomic_store(&page, next);
    atomic_store(&focus, UI_NONE);
    atomic_fetch_add(&ui_revision, 1);
}
static void setup(void) {
    ui_page origin = atomic_load(&page);
    if (origin != UI_PAIRING)
        atomic_store(&setup_parent, origin == UI_CONNECT ? UI_CONNECT :
                                   origin == UI_RESETTING ? UI_MORE : UI_HOME);
    device_stop();
    atomic_store(&mode, DEVICE_SETUP);
    atomic_store(&setup_result, 0);
    mcp_open_setup();
    set_page(UI_PAIRING);
}
static void home(void) {
    device_stop();
    atomic_store(&wifi_setup, false);
    wireless_setup_stop();
    mcp_close_setup();
    atomic_store(&mode, DEVICE_MCP);
    set_page(UI_HOME);
}
static void leave_setup(ui_page next) {
    device_stop();
    atomic_store(&wifi_setup, false);
    wireless_setup_stop();
    mcp_close_setup();
    atomic_store(&mode, DEVICE_MCP);
    set_page(next);
}
static void activate(ui_action action, bool held, uint32_t expected_revision) {
    if (!atomic_load(&screen_ok))
        return;
    switch (action) {
    case UI_START: {
        portENTER_CRITICAL(&permission_lock);
        if (expected_revision == atomic_load(&ui_revision) && atomic_load(&screen_ok) &&
            atomic_load(&page) == UI_HOME && device_mode() == DEVICE_MCP && identity_ready() &&
            mcp_ready() && bridge_input_online() && !wireless_snapshot().setup_ap && !board_stop_pressed() && !actions_busy()) {
            eyes_reset(now_ms());
            atomic_fetch_add(&generation, 1);
            atomic_store(&control_granted, true);
            atomic_fetch_add(&ui_revision, 1);
        }
        portEXIT_CRITICAL(&permission_lock);
        break;
    }
    case UI_STOP:
        device_stop();
        break;
    case UI_SETTINGS_OPEN:
        if (device_mode() == DEVICE_SETUP)
            leave_setup(UI_SETTINGS);
        else { device_stop(); set_page(UI_SETTINGS); }
        break;
    case UI_SETUP_OPEN:
        atomic_store(&wifi_setup, false);
        setup();
        break;
    case UI_INPUT_OPEN:
        device_stop();
        set_page(UI_INPUT);
        break;
    case UI_CONNECT_OPEN:
    case UI_MORE_OPEN:
    case UI_LANGUAGE_OPEN:
    case UI_BLE_RESET_OPEN:
        device_stop();
        set_page(action == UI_CONNECT_OPEN ? UI_CONNECT : action == UI_MORE_OPEN ? UI_MORE :
                 action == UI_LANGUAGE_OPEN ? UI_LANGUAGE : UI_BLE_RESET);
        break;
    case UI_WIFI_SETUP_OPEN:
        device_stop();
        atomic_store(&wifi_setup, true);
        wireless_setup_start(); // Rejections are shown with the recorded stage/error.
        setup();
        break;
    case UI_USB_SELECT:
    case UI_BLE_SELECT:
        device_stop();
        if(wireless_snapshot().setup_ap)break;
        if (bridge_select_input(action==UI_BLE_SELECT ? INPUT_BLE : INPUT_USB))
            atomic_fetch_add(&ui_revision,1);
        break;
    case UI_BLE_RESET_HOLD:
        if (held && atomic_load(&page)==UI_BLE_RESET) {
            device_stop();
            if (!actions_busy()) ble_input_forget();
            atomic_fetch_add(&ui_revision,1);
        }
        break;
    case UI_SETUP_DONE:
        home();
        break;
    case UI_BACK:
        if (device_mode() == DEVICE_SETUP) {
            ui_page origin = atomic_load(&page);
            leave_setup(origin == UI_PAIRING ? atomic_load(&setup_parent) : ui_parent(origin));
        } else set_page(ui_parent(atomic_load(&page)));
        break;
    case UI_CONFIRM_PAIR:
        if (device_mode() == DEVICE_SETUP && atomic_load(&page) == UI_PAIRING) {
            mcp_confirm_setup();
            atomic_fetch_add(&ui_revision, 1);
        }
        break;
    case UI_RESET_OPEN:
        device_stop();
        mcp_close_setup();
        atomic_store(&mode, DEVICE_SETUP);
        atomic_store(&setup_result, 0);
        set_page(UI_RESET);
        break;
    case UI_RESET_HOLD:
        if (held && atomic_load(&page) == UI_RESET) {
            device_stop();
            atomic_store(&setup_result, 0);
            set_page(UI_RESETTING);
            if (!mcp_reset_identity())
                device_setup_finished(false);
        }
        break;
    case UI_ZH_SELECT:
        atomic_store(&language, UI_ZH);
        atomic_fetch_add(&ui_revision, 1);
        break;
    case UI_EN_SELECT:
        atomic_store(&language, UI_EN);
        atomic_fetch_add(&ui_revision, 1);
        break;
    default:
        break;
    }
}
static ui_model model(void) {
    ui_model m = {.page = atomic_load(&page),
                  .language = atomic_load(&language),
                  .mode = device_mode(),
                  .usb_online = bridge_usb_online(),
                  .input_mode = bridge_input_mode(),
                  .input_online = bridge_input_online(),
                  .ble_connected = ble_input_connected(),
                  .ble_ready = ble_input_ready(),
                  .identity_ready = identity_ready(),
                  .mcp_ready = mcp_ready(),
                  .touch_ready = touch_ready() && touch_error() == 0,
                  .input_busy = actions_busy(),
                  .control_granted = device_control_granted(),
                  .revision = atomic_load(&ui_revision),
                  .focus = atomic_load(&focus),
                  .battery_percent = atomic_load(&battery_percent)};
    unsigned feedback = atomic_load(&touch_feedback);
    if (m.revision == atomic_load(&feedback_revision)) {
        m.pressed = (ui_action)(feedback & 255);
        m.hold_percent = feedback >> 8;
    }
    wireless_state network=wireless_snapshot();
    m.wifi_configured=network.configured;
    m.wifi_connected=network.connected;
    m.setup_ap=network.setup_ap;
    m.wifi_setup=atomic_load(&wifi_setup);
    m.wifi_setup_stage=network.setup_stage;
    m.wifi_setup_error=network.setup_error;
    m.http_ready=mcp_http_ready();m.http_error=mcp_http_error();
    snprintf(m.firmware_version,sizeof(m.firmware_version),"%s",esp_app_get_description()->version);
    // Sample only on setup pages; normal animation should not repaint for heap churn.
    if(m.page==UI_CONNECT||(m.page==UI_PAIRING&&m.wifi_setup&&!m.setup_ap)){
        const unsigned internal=MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT;
        m.free_internal_heap=heap_caps_get_free_size(internal);
        m.minimum_internal_heap=heap_caps_get_minimum_free_size(internal);
        m.largest_internal_block=heap_caps_get_largest_free_block(internal);
        m.free_psram=heap_caps_get_free_size(MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        m.reset_reason=esp_reset_reason();m.uptime_seconds=now_ms()/1000;
        m.previous_ap=reset_trace_previous();
    }
    wireless_setup_credentials(m.ap_name,sizeof(m.ap_name),m.ap_password,sizeof(m.ap_password));
    wireless_address(m.wifi_address,sizeof(m.wifi_address));
    if (m.page == UI_PAIRING)
        m.pairing = (ui_pair_state)mcp_pair_snapshot(m.code, sizeof(m.code), m.fingerprint,
                                                     sizeof(m.fingerprint));
    if(m.page==UI_HOME&&m.control_granted)eyes_sample(now_ms(),&m.eyes);
    return m;
}
static bool current(const ui_model *shown) {
    return shown->revision == atomic_load(&ui_revision) && shown->page == (ui_page)atomic_load(&page) &&
           shown->mode == device_mode() && shown->language == (ui_language)atomic_load(&language) &&
           atomic_load(&screen_ok);
}
static bool same_controls(const ui_model *a, const ui_model *b) {
    return a->revision == b->revision && a->page == b->page && a->mode == b->mode &&
           a->language == b->language && a->pairing == b->pairing &&
           a->usb_online == b->usb_online && a->input_mode==b->input_mode &&
           a->input_online==b->input_online && a->ble_ready==b->ble_ready &&
           a->wifi_connected==b->wifi_connected && !strcmp(a->wifi_address,b->wifi_address) &&
           a->setup_ap==b->setup_ap && !strcmp(a->ap_name,b->ap_name) &&
           a->identity_ready == b->identity_ready &&
           a->mcp_ready == b->mcp_ready && !strcmp(a->code, b->code);
}
static void buttons(void *unused) {
    (void)unused;
    bool last_a = false, last_b = false, consumed = false, ca = false, cb = false, a = false,
         b = false;
    uint32_t changed = 0, pressed = 0;
    for (;;) {
        uint32_t now = now_ms();
        bool raw_a = gpio_get_level(2) == 0, raw_b = gpio_get_level(1) == 0;
        // Physical stop has no I2C/rendering dependency and is handled before UI.
        if ((raw_a || raw_b) && (actions_busy() || device_control_granted())) {
            device_stop();
            consumed = true;
        }
        if (raw_a != ca || raw_b != cb) {
            ca = raw_a;
            cb = raw_b;
            changed = now;
        }
        if ((uint32_t)(now - changed) >= 20) {
            a = ca;
            b = cb;
        }
        if ((a || b) && !last_a && !last_b) {
            pressed = now;
        }
        if (device_mode() == DEVICE_MCP && (!bridge_input_online() || !atomic_load(&screen_ok)) &&
            device_control_granted())
            device_stop();
        ui_model shown = {0};
        bool visible = ui_displayed(&shown) && current(&shown);
        if (!consumed && visible && b && !a &&
            (shown.focus == UI_RESET_HOLD || shown.focus == UI_BLE_RESET_HOLD) && pressed &&
            (uint32_t)(now - pressed) >= 2000) {
            activate(shown.focus, true, shown.revision);
            consumed = true;
        } else if (!consumed && visible && a && !b && pressed && (uint32_t)(now - pressed) >= 700) {
            activate(UI_SETTINGS_OPEN, false, shown.revision);
            consumed = true;
        } else if (!consumed && visible && !a && last_a) {
            atomic_store(&focus, ui_next_focus(&shown));
        } else if (!consumed && visible && !b && last_b && shown.focus != UI_NONE) {
            activate(shown.focus, false, shown.revision);
        }
        if (!a && !b && !raw_a && !raw_b) {
            consumed = false;
            pressed = 0;
        }
        last_a = a;
        last_b = b;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
static void touch_input(void *unused) {
    (void)unused;
    bool down = false, blocked = true, consumed = false;
    ui_model pressed_model = {0};
    ui_action target = UI_NONE;
    uint32_t started = 0;
    uint16_t last_x = 0, last_y = 0;
    for (;;) {
        touch_point point;
        ui_model shown = {0};
        bool read_ok = touch_read(&point);
        if(device_control_granted())
            avatar_touch(read_ok&&point.pressed,point.x,point.y,now_ms());
        else avatar_touch(false,0,0,now_ms());
        atomic_store(&touch_pressed, !read_ok || point.pressed);
        if (!read_ok) {
            if (device_control_granted() || actions_busy())
                device_stop();
            down = false;
            blocked = true;
            target = UI_NONE;
        } else if (!point.pressed) {
            if (down && !consumed && !blocked && ui_displayed(&shown) && current(&shown) &&
                same_controls(&shown, &pressed_model) && ui_target_contains(&shown, target, last_x, last_y, 10) &&
                target != UI_RESET_HOLD && target != UI_BLE_RESET_HOLD)
                activate(target, false, shown.revision);
            down = false;
            consumed = false;
            blocked = false;
            target = UI_NONE;
        } else if (!blocked) {
            last_x = point.x;
            last_y = point.y;
            if (!down) {
                down = true;
                started = now_ms();
                consumed = false;
                if (device_control_granted() || actions_busy()) {
                    // The control face has no touch targets. Consume the entire
                    // contact so a physical/MCP stop cannot turn it into Start.
                    consumed = true;
                } else if (ui_displayed(&shown) && current(&shown) && !board_stop_pressed()) {
                    pressed_model = shown;
                    target = ui_hit(&shown, point.x, point.y);
                } else
                    consumed = true;
            } else if (!consumed) {
                if (board_stop_pressed() || !ui_displayed(&shown) || !current(&shown) ||
                    !same_controls(&shown, &pressed_model) ||
                    !ui_target_contains(&shown, target, point.x, point.y,
                        target == UI_RESET_HOLD || target == UI_BLE_RESET_HOLD ? 0 : 10))
                    consumed = true;
                else if ((target == UI_RESET_HOLD || target == UI_BLE_RESET_HOLD) &&
                         (uint32_t)(now_ms() - started) >= 2000) {
                    activate(target, true, shown.revision);
                    consumed = true;
                }
            }
        }
        // Visual-only feedback cannot change the grant revision or retarget a gesture.
        unsigned feedback = 0;
        if (down && !consumed && !blocked && target != UI_NONE) {
            unsigned progress = 0;
            if (target == UI_RESET_HOLD || target == UI_BLE_RESET_HOLD) {
                progress = (uint32_t)(now_ms() - started) / 20;
                if (progress > 100) progress = 100;
            }
            atomic_store(&feedback_revision, pressed_model.revision);
            feedback = (unsigned)target | (progress << 8);
        }
        atomic_store(&touch_feedback, feedback);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
static void screen(void *unused) {
    (void)unused;
    display_init();
    ui_model m = model();
    atomic_store(&screen_ok, ui_present(&m));
    if (touch_init())
        configASSERT(xTaskCreate(touch_input, "touch-input", UI_INPUT_STACK_BYTES, NULL, 3, NULL) == pdPASS);
    else
        ESP_LOGW("touch", "touch unavailable; physical key navigation remains active");
    imu_start();
    avatar_state face;
    avatar_timing timing={0};
    uint32_t dma_transfers=0,dma_pixels=0;
    bool face_active=false;
    ui_model previous = {0};
    bool have_previous = false;
    bool initial_network_checked=false;
    bool previous_ap=false;
    uint32_t last_battery = 0;
    bool battery_sampled = false;
    ui_language saved = atomic_load(&language);
    input_mode_t saved_input=bridge_input_mode();
    for (;;) {
        uint32_t frame_started=now_ms();
        wireless_state network=wireless_snapshot();
        if(network.ready&&!initial_network_checked){
            initial_network_checked=true;
            if(!network.configured){device_stop();atomic_store(&wifi_setup,true);wireless_setup_start();setup();}
        }
        // A previous AP closing must not cancel a newly queued setup or hide
        // its startup error. Genuine expiry still returns to Home.
        if(previous_ap&&!network.setup_ap&&device_mode()==DEVICE_SETUP&&!wireless_setup_requested()&&!network.setup_error)home();
        previous_ap=network.setup_ap;
        if (atomic_load(&page) == UI_RESETTING && atomic_load(&setup_result)) {
            if (atomic_load(&setup_result) == 1)
                setup();
            else
                set_page(UI_RESET);
        }
        uint32_t now = now_ms();
        if (atomic_load(&page) == UI_HOME && !device_control_granted() && !actions_busy() &&
            !atomic_load(&touch_pressed) && !board_stop_pressed() &&
            (!battery_sampled || (uint32_t)(now-last_battery) >= 15000)) {
            uint16_t millivolts;
            int percent = board_battery_voltage(&millivolts) ? battery_percent_from_mv(millivolts) : -1;
            atomic_store(&battery_percent, percent);
            last_battery = now;
            battery_sampled = true;
        }
        m = model();
        bool show_face=m.page==UI_HOME&&m.control_granted;
        if(show_face){
            if(!face_active){avatar_reset(&face,now);imu_recenter();}
            avatar_step(&face,now,&m.eyes,imu_snapshot(now),&m.avatar);
            m.avatar_valid=true;
        }
        face_active=show_face;
        uint32_t paint_us=0;
        if (!have_previous || memcmp(&m, &previous, sizeof(m))) {
            uint32_t started_us=(uint32_t)esp_timer_get_time();
            atomic_store(&screen_ok, ui_present(&m));
            if(ui_painted_last_frame())paint_us=(uint32_t)esp_timer_get_time()-started_us;
            if(show_face){display_paint_metrics stats=display_last_paint_metrics();dma_transfers+=stats.transfers;dma_pixels+=stats.pixels;}
            previous = m;
            have_previous = true;
        }
        if(show_face){
            avatar_timing_report report;
            if(avatar_timing_update(&timing,(uint32_t)esp_timer_get_time(),paint_us,&report)){
                ESP_LOGI("avatar","updates=%u paints=%u update_fps=%u.%03u paint_mean_us=%u paint_max_us=%u dma_transfers=%u dma_pixels=%u",
                    (unsigned)report.updates,(unsigned)report.paints,(unsigned)(report.update_fps_milli/1000),
                    (unsigned)(report.update_fps_milli%1000),(unsigned)report.paint_mean_us,(unsigned)report.paint_max_us,
                    (unsigned)dma_transfers,(unsigned)dma_pixels);
                dma_transfers=dma_pixels=0;
            }
        }else {memset(&timing,0,sizeof(timing));dma_transfers=dma_pixels=0;}
        // Preference writes occur only in the low-priority screen task, never in stop handling.
        if (m.language != saved) {
            if (!ui_language_save(m.language))
                ESP_LOGW("ui", "language preference could not be saved");
            saved = m.language;
        }
        if (m.input_mode!=saved_input) {
            if (!ui_input_save(m.input_mode)) ESP_LOGW("ui","input preference could not be saved");
            saved_input=m.input_mode;
        }
        uint32_t spent=now_ms()-frame_started;
        vTaskDelay(pdMS_TO_TICKS(show_face?(spent<17?17-spent:1):100));
    }
}
void device_init(void) {
    atomic_store(&mode, DEVICE_MCP);
    ui_init();
    atomic_store(&language, ui_language_load());
    bridge_select_input(ui_input_load());
    configASSERT(xTaskCreate(buttons, "device-buttons", UI_INPUT_STACK_BYTES, NULL, 6, NULL) == pdPASS);
    configASSERT(xTaskCreate(screen, "device-screen", 8192, NULL, 1, NULL) == pdPASS);
}

// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
typedef enum {
    WIFI_SETUP_IDLE, WIFI_SETUP_QUEUED, WIFI_SETUP_RELEASE, WIFI_SETUP_BLE,
    WIFI_SETUP_MODE, WIFI_SETUP_CONFIG, WIFI_SETUP_START, WIFI_SETUP_READY
} wireless_setup_stage;
typedef struct {
    bool configured, connected, setup_ap, ready;
    uint32_t address, netmask, generation, ap_address;
    int error;
    wireless_setup_stage setup_stage;
    int setup_error;
} wireless_state;
void wireless_init(void);
wireless_state wireless_snapshot(void);
void wireless_address(char *out, size_t size);
// Empty SSID/password forgets all networks; callers enforce physical setup.
esp_err_t wireless_configure(const char *ssid, const char *password);
esp_err_t wireless_forget(const char *ssid);
unsigned wireless_saved_names(char names[5][33]);
esp_err_t wireless_setup_start(void);
bool wireless_setup_requested(void);
void wireless_setup_stop(void);
void wireless_setup_credentials(char *name,size_t name_size,char *password,size_t password_size);
#define WIRELESS_SCAN_LIMIT 20
typedef struct { char ssid[33]; int8_t rssi; bool open; } wireless_network;
typedef struct {
    bool busy, done;
    int error;
    unsigned count;
    wireless_network networks[WIRELESS_SCAN_LIMIT];
} wireless_scan_result;
esp_err_t wireless_scan_begin(void);
void wireless_scan_snapshot(wireless_scan_result *out);

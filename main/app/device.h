// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum { DEVICE_MCP, DEVICE_SETUP } device_mode_t;
void device_init(void);
device_mode_t device_mode(void);
bool device_control_allowed(void);
uint32_t device_generation(void);
bool device_control_granted(void);
void device_stop(void);
void device_setup_finished(bool ok);
const char *device_ui_language(void);

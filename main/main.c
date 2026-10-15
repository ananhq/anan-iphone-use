// SPDX-License-Identifier: Apache-2.0
#include "bridge.h"
#include "actions.h"
#include "device.h"
#include "mcp.h"
#include "wireless.h"
#include "input_backends.h"
#include "discovery.h"
#include "reset_trace.h"

void app_main(void) {
    reset_trace_init();
    board_init();
    bridge_usb_init();
    bridge_input_init();
    actions_init();
    mcp_init();
    device_init();
    wireless_init();
    ble_input_init();
    discovery_init();
}

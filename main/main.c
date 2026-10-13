// SPDX-License-Identifier: Apache-2.0
#include "bridge.h"
#include "actions.h"
#include "device.h"
#include "mcp.h"

void app_main(void) {
    board_init();
    bridge_usb_init();
    actions_init();
    mcp_init();
    device_init();
}

// SPDX-License-Identifier: Apache-2.0
#include "battery.h"
int battery_percent_from_mv(uint16_t millivolts) {
    if (millivolts < 2500 || millivolts > 4500)
        return -1;
    if (millivolts <= 3300)
        return 0;
    if (millivolts >= 4200)
        return 100;
    // 900mV spans twenty 5% steps; rounding avoids spurious 1% precision.
    return ((millivolts - 3300 + 22) / 45) * 5;
}

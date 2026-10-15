// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdint.h>
// Uncalibrated 3.3–4.2V linear estimate rounded to 5%; -1 for invalid samples.
int battery_percent_from_mv(uint16_t millivolts);

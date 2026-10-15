// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tilt.h"
void imu_start(void);
void imu_recenter(void);
tilt_gaze imu_snapshot(uint32_t now);

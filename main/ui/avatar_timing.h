// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {uint32_t window,updates,paints,paint_total,paint_max;bool started;} avatar_timing;
typedef struct {uint32_t updates,paints,paint_mean_us,paint_max_us,update_fps_milli;} avatar_timing_report;
bool avatar_timing_update(avatar_timing *state,uint32_t now_us,uint32_t paint_us,avatar_timing_report *report);

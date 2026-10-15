// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct { float ax,ay,az,gx,gy,gz; } tilt_sample; // g and degrees/second
typedef struct { float x,y;bool ready; } tilt_gaze;
typedef struct { float neutral_x,neutral_y,x,y;uint32_t last;bool ready; } tilt_state;
void tilt_reset(tilt_state *state);
tilt_gaze tilt_update(tilt_state *state,const tilt_sample *sample,uint32_t now);

// SPDX-License-Identifier: Apache-2.0
// Predefined gravity/gyro synthetic trajectories, no IMU hardware.
#include "tilt.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void) {
    tilt_state s;tilt_reset(&s);tilt_gaze out;
    tilt_sample flat={.az=1};out=tilt_update(&s,&flat,0);assert(out.ready&&fabsf(out.x)<.001f);
    for(unsigned t=25;t<10000;t+=25)out=tilt_update(&s,&flat,t);
    assert(fabsf(out.x)<.01f&&fabsf(out.y)<.01f);
    tilt_sample right={.ay=-.5f,.az=.8660254f};
    tilt_state fast;tilt_reset(&fast);tilt_update(&fast,&flat,0);
    for(unsigned t=25;t<=150;t+=25)out=tilt_update(&fast,&right,t);
    assert(out.x>.65f); // accelerometer-only fallback settles promptly without gyro motion
    for(unsigned t=10000;t<12000;t+=25)out=tilt_update(&s,&right,t);
    assert(out.x>.65f&&fabsf(out.y)<.01f); // sensor Y -> screen X
    tilt_sample down={.ax=-.5f,.az=.8660254f};
    for(unsigned t=12000;t<14000;t+=25)out=tilt_update(&s,&down,t);
    assert(out.y>.65f&&fabsf(out.x)<.1f); // sensor X -> screen Y
    flat.gx=.2f;for(unsigned t=14000;t<20000;t+=25)out=tilt_update(&s,&flat,t);
    assert(fabsf(out.x)<.03f&&fabsf(out.y)<.03f);
    tilt_sample invalid={0};assert(!tilt_update(&s,&invalid,20025).ready);
    invalid.ax=NAN;assert(!tilt_update(&s,&invalid,20050).ready);
    invalid=(tilt_sample){.az=4};assert(!tilt_update(&s,&invalid,20075).ready);
    flat.gx=2000;for(unsigned t=20100;t<21000;t+=25){out=tilt_update(&s,&flat,t);assert(isfinite(out.x)&&fabsf(out.x)<=1&&fabsf(out.y)<=1);}
    tilt_reset(&s);out=tilt_update(&s,&right,UINT32_MAX-20);assert(out.ready&&fabsf(out.x)<.01f);
    out=tilt_update(&s,&right,20);assert(out.ready&&fabsf(out.x)<.01f);
    puts("Tilt: screen axes, neutral, held tilt, gyro drift/bounds, invalid acceleration and clock wrap: passed");
}

// SPDX-License-Identifier: Apache-2.0
// Predefined synthetic update/paint timing windows; not a board measurement.
#include "avatar_timing.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    avatar_timing s={0};avatar_timing_report report;
    assert(!avatar_timing_update(&s,0,1000,&report));
    for(unsigned i=1;i<150;i++)assert(!avatar_timing_update(&s,i*33000,i%2?0:3000,&report));
    assert(avatar_timing_update(&s,5000000,5000,&report));
    assert(report.updates==151&&report.paints==76&&report.paint_max_us==5000);
    assert(report.paint_mean_us>2900&&report.paint_mean_us<3100);
    assert(report.update_fps_milli==30200);
    puts("Avatar timing: five-second update/paint counts, zero-paint ticks, mean/max and measured cadence: passed");
}

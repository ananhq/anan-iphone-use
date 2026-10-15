// SPDX-License-Identifier: Apache-2.0
// Acceptance defined before implementation. No I2C/device access.
#include "battery.h"
#include <assert.h>
#include <stdio.h>
int main(void){
    assert(battery_percent_from_mv(0)==-1);
    assert(battery_percent_from_mv(2499)==-1);
    assert(battery_percent_from_mv(4501)==-1);
    assert(battery_percent_from_mv(3300)==0);
    assert(battery_percent_from_mv(4200)==100);
    assert(battery_percent_from_mv(3750)==50);
    assert(battery_percent_from_mv(3975)==75);
    int previous=0;for(unsigned mv=2500;mv<=4500;mv++){
        int p=battery_percent_from_mv(mv);assert(p>=previous&&p<=100&&p%5==0);previous=p;
    }
    puts("Battery invalid ranges, endpoints, coarse rounding and monotonic estimate: passed");
}

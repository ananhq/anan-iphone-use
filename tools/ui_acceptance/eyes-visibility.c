// SPDX-License-Identifier: Apache-2.0
// Defined before the stronger-feedback implementation; synthetic render cadence.
#include "eyes.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static bool distinct(eyes_pose p) {
    for(int i=0;i<2;i++)
        if(abs(p.eye[i].ry-72)>=15||abs(p.eye[i].x-(i?305:161))>=22||abs(p.eye[i].y-233)>=22)
            return true;
    return false;
}
int main(void) {
    for(int k=ACTION_TAP;k<=ACTION_CONSUMER;k++) {
        action a={.kind=k,.x=32768,.y=32768,.vertical=-1};
        eyes_reset(0);eyes_begin(&a,0);
        eyes_pose p;eyes_sample(0,&p);
        if(!distinct(p)){fprintf(stderr,"Action %d lacks a distinct first pose\n",k);return 1;}
        eyes_end(true,20); // An input can complete before the next full frame.
        for(unsigned phase=0;phase<160;phase+=40) {
            unsigned seen=0;
            for(unsigned t=phase;t<=880;t+=160) {
                eyes_sample(t,&p);if(distinct(p))seen++;
            }
            if(seen<4){fprintf(stderr,"Action %d too brief/subtle at 160ms render cadence\n",k);return 1;}
        }
        eyes_sample(1500,&p);assert(!distinct(p));
    }
    puts("Action feedback: distinct first pose and >=4 visible samples at 160ms cadence, all input families, then idle: passed");
}

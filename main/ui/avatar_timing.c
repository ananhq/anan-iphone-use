// SPDX-License-Identifier: Apache-2.0
#include "avatar_timing.h"
bool avatar_timing_update(avatar_timing *s,uint32_t now,uint32_t paint,avatar_timing_report *out){
    if(!s->started){s->started=true;s->window=now;}
    s->updates++;
    if(paint){s->paints++;s->paint_total+=paint;if(paint>s->paint_max)s->paint_max=paint;}
    uint32_t elapsed=now-s->window;
    if(elapsed<5000000)return false;
    *out=(avatar_timing_report){s->updates,s->paints,s->paints?s->paint_total/s->paints:0,s->paint_max,
        (uint32_t)((uint64_t)s->updates*1000000000/elapsed)};
    s->window=now;s->updates=s->paints=s->paint_total=s->paint_max=0;
    return true;
}

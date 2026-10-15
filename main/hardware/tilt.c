// SPDX-License-Identifier: Apache-2.0
#include "tilt.h"
#include <math.h>
#include <string.h>
static float clamp(float x,float limit){return fmaxf(-limit,fminf(limit,x));}
static float wrap(float x){while(x>3.1415927f)x-=6.2831853f;while(x< -3.1415927f)x+=6.2831853f;return x;}
void tilt_reset(tilt_state *s){memset(s,0,sizeof(*s));}
tilt_gaze tilt_update(tilt_state *s,const tilt_sample *p,uint32_t now){
    float norm=p->ax*p->ax+p->ay*p->ay+p->az*p->az;
    if(!isfinite(norm)||!isfinite(p->gx)||!isfinite(p->gy)||norm<.42f||norm>1.96f)
        return (tilt_gaze){0};
    // Factory screen mapping: sensor Y is horizontal, sensor X is vertical.
    float x=atan2f(-p->ay,p->az),y=atan2f(-p->ax,sqrtf(p->ay*p->ay+p->az*p->az));
    if(!s->ready){s->neutral_x=x;s->neutral_y=y;s->last=now;s->ready=true;}
    float dt=fminf((uint32_t)(now-s->last)*.001f,.1f);s->last=now;
    float gyro_x=fabsf(p->gx)<.8f?0:-p->gx*.017453293f;
    float gyro_y=fabsf(p->gy)<.8f?0:p->gy*.017453293f;
    float amount=1-expf(-dt/.12f);
    s->x=clamp(s->x+gyro_x*dt,1.1f);s->y=clamp(s->y+gyro_y*dt,1.1f);
    s->x+=wrap(wrap(x-s->neutral_x)-s->x)*amount;
    s->y+=wrap(wrap(y-s->neutral_y)-s->y)*amount;
    return (tilt_gaze){.x=clamp(s->x/.5235988f,1),.y=clamp(s->y/.5235988f,1),.ready=true};
}

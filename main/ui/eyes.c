// SPDX-License-Identifier: Apache-2.0
#include "eyes.h"
#include <math.h>
#include <stdatomic.h>
#include <string.h>

enum { EYES_IDLE=-1, EYES_INTERRUPTED=ACTION_CONSUMER+1 };
static atomic_uint serial, origin, started, ended, pointer, event_id;
static atomic_int kind=ATOMIC_VAR_INIT(EYES_IDLE), vertical, horizontal;
static atomic_bool running;
bool eyes_event_snapshot(eyes_event *out) {
    unsigned seq=atomic_load(&serial);
    if(seq&1)return false;
    *out=(eyes_event){seq,atomic_load(&started),atomic_load(&ended),atomic_load(&pointer),atomic_load(&event_id),
        atomic_load(&kind),atomic_load(&vertical),atomic_load(&horizontal),atomic_load(&running)};
    return seq==atomic_load(&serial);
}
// Lifecycle writes are serialized by the single action worker and Start's !busy
// gate. End is published before busy clears. Readers never spin or block input.
void eyes_reset(uint32_t now) {
    atomic_fetch_add(&serial,1);
    atomic_store(&origin,now);
    atomic_store(&kind,EYES_IDLE);
    atomic_store(&running,false);
    atomic_fetch_add(&serial,1);
}
void eyes_begin(const action *a,uint32_t now) {
    atomic_fetch_add(&serial,1);
    atomic_fetch_add(&event_id,1);
    atomic_store(&started,now);
    atomic_store(&kind,a->kind);
    atomic_store(&vertical,a->vertical);
    atomic_store(&horizontal,a->horizontal);
    eyes_pointer(a->x,a->y);
    atomic_store(&running,true);
    atomic_fetch_add(&serial,1);
}
void eyes_pointer(uint16_t x,uint16_t y) {
    atomic_store(&pointer,((uint32_t)y<<16)|x);
}
void eyes_end(bool succeeded,uint32_t now) {
    atomic_fetch_add(&serial,1);
    atomic_store(&ended,now);
    if(!succeeded) {
        atomic_store(&kind,EYES_INTERRUPTED);
        atomic_store(&started,now);
    }
    atomic_store(&running,false);
    atomic_fetch_add(&serial,1);
}
static float pulse(unsigned phase,unsigned center,unsigned width) {
    unsigned distance=phase>center?phase-center:center-phase;
    return distance<width?1.0f-(float)distance/width:0.0f;
}
void eyes_sample(uint32_t now,eyes_pose *out) {
    uint32_t seq=atomic_load(&serial), idle_age=now-atomic_load(&origin);
    int k=atomic_load(&kind),v=atomic_load(&vertical),h=atomic_load(&horizontal);
    uint32_t age=now-atomic_load(&started), tail=now-atomic_load(&ended);
    bool active=atomic_load(&running);
    uint32_t xy=atomic_load(&pointer);
    if((seq&1)||seq!=atomic_load(&serial)) k=EYES_IDLE;
    float weight=1;
    if(!active&&age>=900&&tail>=320) {
        uint32_t fade=age-900<tail-320?age-900:tail-320;
        weight=fade<240?1.0f-(float)fade/240:0;
    }
    float dx=0,dy=2*sinf((idle_age%4200)*6.2831853f/4200);
    float left=1,right=1, left_y=0,right_y=0;
    int radius_x=52;
    if(k==EYES_IDLE||weight<=0) {
        left=right=1-.95f*pulse(idle_age%4200,3080,100);
    } else {
        float wave=sinf((age%480)*6.2831853f/480);
        switch(k) {
        case ACTION_DRAG:case ACTION_DRAG_HOLD:case ACTION_PATH:
        case ACTION_TAP:case ACTION_LONG_PRESS:case ACTION_DOUBLE_TAP:
            dx=((int)(xy&65535)-32768)*42.0f/32768;
            dy=((int)(xy>>16)-32768)*34.0f/32768;
            // A distinct pose lasts between peaks, so a slow full-frame redraw
            // cannot miss the entire reaction of an already-completed short tap.
            if(k==ACTION_DRAG||k==ACTION_DRAG_HOLD||k==ACTION_PATH) {
                left=right=.64f+.04f*wave;radius_x=56;
            } else if(k==ACTION_LONG_PRESS) {
                left=right=.56f-.06f*wave;radius_x=56;
            } else {
                float blink=k==ACTION_DOUBLE_TAP?
                    fmaxf(pulse(age%700,110,110),pulse(age%700,360,110)):
                    pulse(age%600,150,150);
                left=right=.58f-.48f*blink;radius_x=58;
            }
            break;
        case ACTION_TYPE:
            dy=-12*fabsf(wave);left_y=-18*wave;right_y=18*wave;
            left=.72f-.24f*wave;right=.72f+.24f*wave;
            break;
        case ACTION_SCROLL:
            // Wheel counts are not pointer coordinates; use direction only.
            dx=(h<0?-1:h>0)*24.0f;dy=-(v<0?-1:v>0)*(24+9*wave);
            left=right=.68f;
            break;
        case ACTION_KEY:case ACTION_KEYBOARD:case ACTION_CONSUMER:
            left=1.08f;right=.2f+.28f*(1-pulse(age%700,180,180));dy=-6;
            break;
        case EYES_INTERRUPTED:
            left=.65f;right=.42f;dy=8;break;
        default:break;
        }
        dx*=weight;dy*=weight;left_y*=weight;right_y*=weight;
        radius_x=52+(int)lroundf((radius_x-52)*weight);
        left=1-(1-left)*weight;right=1-(1-right)*weight;
    }
    *out=(eyes_pose){.eye={{161+(int)lroundf(dx),233+(int)lroundf(dy+left_y),radius_x,4+(int)lroundf(68*left)},
                            {305+(int)lroundf(dx),233+(int)lroundf(dy+right_y),radius_x,4+(int)lroundf(68*right)}},
                            .action=k!=EYES_IDLE&&weight>0};
}
void eyes_paint_strip(const eyes_pose *pose,uint16_t *pixels,int top) {
    memset(pixels,0,466*16*sizeof(*pixels));
    for(unsigned i=0;i<2;i++) {
        eye_shape e=pose->eye[i];
        if(e.rx<1||e.ry<1)continue;
        for(int y=top;y<top+16&&y<466;y++) {
            int dy=y-e.y;
            if(dy < -e.ry||dy>e.ry)continue;
            // Row span of an ellipse; inward rounding leaves every lit pixel
            // inside the shape. RGB565 0xe73f, byte-swapped for the panel.
            int span=(int)sqrtf((float)e.rx*e.rx*(e.ry*e.ry-dy*dy)/(e.ry*e.ry));
            for(int x=e.x-span;x<=e.x+span;x++)
                if(x>=0&&x<466)pixels[(y-top)*466+x]=0x3fe7;
        }
    }
}

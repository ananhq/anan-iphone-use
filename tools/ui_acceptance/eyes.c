// SPDX-License-Identifier: Apache-2.0
// Acceptance defined before the eye animation implementation. Synthetic time only.
#include "eyes.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static eyes_pose sample(uint32_t now) { eyes_pose p;eyes_sample(now,&p);return p; }
static void bounds(eyes_pose p) {
    for(unsigned i=0;i<2;i++) {
        assert(p.eye[i].rx>=4&&p.eye[i].rx<=64&&p.eye[i].ry>=4&&p.eye[i].ry<=84);
        for(int x=-1;x<=1;x+=2)for(int y=-1;y<=1;y+=2) {
            int dx=p.eye[i].x+x*p.eye[i].rx-233,dy=p.eye[i].y+y*p.eye[i].ry-233;
            assert(dx*dx+dy*dy<233*233);
        }
    }
    uint16_t pixels[466*16];unsigned ink=0;
    for(int top=0;top<466;top+=16) {
        memset(pixels,0xa5,sizeof(pixels));eyes_paint_strip(&p,pixels,top);
        for(int dy=0;dy<16;dy++)for(int x=0;x<466;x++) {
            uint16_t color=pixels[dy*466+x];assert(color==0||color==0x3fe7);
            if(color) {
                int y=top+dy;bool inside=false;
                for(int i=0;i<2;i++) {
                    int dx=x-p.eye[i].x,d=y-p.eye[i].y;
                    if(dx*dx*p.eye[i].ry*p.eye[i].ry+d*d*p.eye[i].rx*p.eye[i].rx<=
                       p.eye[i].rx*p.eye[i].rx*p.eye[i].ry*p.eye[i].ry)inside=true;
                }
                assert(inside&&y<466);ink++;
            }
        }
    }
    assert(ink>100&&ink<50000);
}
int main(void) {
    eyes_reset(0);eyes_pose idle=sample(0);bounds(idle);
    assert(idle.eye[0].x<233&&idle.eye[1].x>233&&idle.eye[0].ry>50);
    bool blink=false;for(unsigned t=0;t<4200;t+=40){eyes_pose p=sample(t);bounds(p);if(p.eye[0].ry<15)blink=true;}assert(blink);
    action a={.kind=ACTION_DRAG,.x=0,.y=0};eyes_begin(&a,5000);eyes_pointer(0,0);
    eyes_pose left=sample(5100);eyes_pointer(65535,65535);eyes_pose right=sample(5100);
    assert(right.eye[0].x>left.eye[0].x&&right.eye[0].y>left.eye[0].y);
    eyes_end(true,5200);eyes_pose linger=sample(5250);assert(linger.eye[0].x>idle.eye[0].x);
    eyes_pose settled=sample(6400);assert(settled.eye[0].x==idle.eye[0].x);
    for(int k=ACTION_TAP;k<=ACTION_CONSUMER;k++) {
        a.kind=k;a.vertical=-1;a.horizontal=1;eyes_begin(&a,6000);
        for(unsigned t=6000;t<7000;t+=80)bounds(sample(t));
        eyes_end(true,7000);bounds(sample(7100));
    }
    // Wheel magnitude controls input speed, never an unbounded eye displacement.
    const int amounts[]={-127,-3,-1,0,1,3,127};
    for(unsigned i=0;i<sizeof(amounts)/sizeof(amounts[0]);i++) {
        a.kind=ACTION_SCROLL;a.vertical=amounts[i];a.horizontal=-amounts[i];
        eyes_begin(&a,7200);bounds(sample(7280));
    }
    a.kind=ACTION_TYPE;eyes_begin(&a,8000);eyes_pose type=sample(8080);
    a.kind=ACTION_KEYBOARD;eyes_begin(&a,8080);eyes_pose key=sample(8200);
    assert(memcmp(&type,&key,sizeof(key))&&key.eye[0].ry>key.eye[1].ry);
    eyes_end(false,8250);eyes_pose fail=sample(8280);assert(fail.eye[0].ry!=fail.eye[1].ry);
    eyes_reset(8280);eyes_pose reset=sample(8280);assert(!memcmp(&idle,&reset,sizeof(idle)));
    eyes_reset(UINT32_MAX-100);eyes_pose wrap=sample(50);bounds(wrap);
    a.kind=ACTION_TAP;eyes_begin(&a,UINT32_MAX-20);eyes_end(true,10);bounds(sample(50));
    puts("Eyes: bounds, pure black background, blink, pointer, action families, linger/replacement/reset and clock wrap: passed");
}

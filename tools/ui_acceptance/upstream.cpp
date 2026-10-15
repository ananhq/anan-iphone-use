// SPDX-License-Identifier: Apache-2.0
// Acceptance fixture defined before the upstream runtime port; host only.
#include "avatar.h"
#include "avatar_native.hpp"
#include "vendor/stopwatch_avatar/avatar_engine.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <set>
static uint16_t full[466*466],partial[466*466],strip[466*16];
static unsigned checksum(){unsigned n=2166136261u;for(auto p:full)n=(n^p)*16777619u;return n;}
static void paint(const avatar_pose &p){for(int y=0;y<466;y+=16){avatar_paint_strip(&p,strip,y);memcpy(full+y*466,strip,(y+16<466?16:466-y)*466*2);}}
static void cropped(const avatar_pose &p){for(unsigned e=0;e<2;e++){auto r=avatar_damage_eye(&p,&p,e);assert(r.x>=0&&r.y>=0&&r.x+r.w<=466&&r.y+r.h<=466);for(int y=r.y;y<r.y+r.h;y+=16){avatar_paint_strip(&p,strip,y);for(int row=0;row<16&&y+row<r.y+r.h;row++)memcpy(partial+(y+row)*466+r.x,strip+row*466+r.x,r.w*2);}}}
int main(){
    std::set<unsigned> images;
    AvatarEngine engine;avatar_native_clock(0);assert(engine.begin());
    bool saw_brow=false;
    for(unsigned id=0;id<(unsigned)ExpressionId::Count;id++){
        engine.show((ExpressionId)id,id*5000,true);
        for(unsigned dt=0;dt<1700;dt+=17){
            unsigned t=id*5000+dt;avatar_native_clock(t);engine.update(t);
            avatar_pose p=avatar_native_frame();assert(!avatar_native_overflow());
            assert(avatar_native_command_count()>0&&avatar_native_command_count()<=160);
            saw_brow|=avatar_native_has_brow();paint(p);cropped(p);
            assert(!memcmp(full,partial,sizeof(full)));
        }
        images.insert(checksum());
    }
    assert(images.size()>=10&&saw_brow);
    eyes_reset(0);avatar_state state;avatar_pose p;eyes_pose legacy={};
    avatar_reset(&state,0);avatar_step(&state,0,&legacy,{},&p);
    action a={};a.kind=ACTION_TAP;eyes_begin(&a,20);eyes_end(true,90);
    avatar_step(&state,100,&legacy,{},&p);assert(avatar_native_expression()==(unsigned)ExpressionId::Happy);
    for(unsigned t=117;t<400;t+=17)avatar_step(&state,t,&legacy,{},&p);
    assert(avatar_native_expression()==(unsigned)ExpressionId::Happy);
    avatar_reset(&state,500);avatar_step(&state,500,&legacy,{},&p);
    assert(avatar_native_expression()==(unsigned)ExpressionId::Idle);
    avatar_touch(true,230,230,600);avatar_step(&state,600,&legacy,{},&p);
    avatar_touch(false,230,230,680);avatar_step(&state,680,&legacy,{},&p);
    assert(avatar_native_expression()==(unsigned)ExpressionId::Happy);
    avatar_touch(true,230,230,750);avatar_step(&state,750,&legacy,{},&p);
    avatar_touch(false,230,230,820);avatar_step(&state,820,&legacy,{},&p);
    assert(avatar_native_expression()==(unsigned)ExpressionId::Surprised);
    avatar_touch(true,230,230,1100);avatar_step(&state,1100,&legacy,{},&p);
    avatar_step(&state,1760,&legacy,{},&p);
    assert(avatar_native_expression()==(unsigned)ExpressionId::Angry);
    avatar_touch(false,230,230,1800);avatar_step(&state,1800,&legacy,{},&p);
    avatar_touch(true,230,230,2000);avatar_step(&state,2000,&legacy,{},&p);
    avatar_touch(true,230,120,2050);avatar_step(&state,2050,&legacy,{},&p);
    assert(avatar_native_expression()==(unsigned)ExpressionId::Surprised);
    // Overflow is explicit and cannot publish a partially collected frame.
    M5.Display.startWrite();for(int i=0;i<200;i++)M5.Display.fillCircle(100,100,4,0xffff);M5.Display.endWrite();
    assert(avatar_native_overflow());
    puts("Upstream expressions, brows, dirty/full raster, action lifecycle and local gestures: passed");
}

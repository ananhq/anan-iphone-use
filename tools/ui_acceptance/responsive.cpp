// SPDX-License-Identifier: Apache-2.0
// Defined before responsive runtime changes. Actual adapter and engine, no device.
#include "avatar_native.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cassert>
#include <cstdio>
// Read engine state in this fixture only, without adding a firmware/MCP API.
#define private public
#include "vendor/stopwatch_avatar/avatar_engine.h"
#undef private
#include "avatar.cpp"
int main(){
    AvatarEngine one,many;avatar_native_clock(1000);one.begin();many.begin();
    one.setTiltTarget(.8f,-.3f);many.setTiltTarget(.8f,-.3f);
    one.updateInteraction(1000);many.updateInteraction(1000);
    one.updateInteraction(1200);
    for(unsigned now=1040;now<=1200;now+=40)many.updateInteraction(now);
    assert(std::abs(one.interactionX_-many.interactionX_)<.01f);
    assert(std::abs(one.headYaw_-many.headYaw_)<.01f);
    assert(one.interactionX_>one.tiltTargetX_*.9f);
    one.updateInteraction(22000);assert(std::isfinite(one.interactionX_)&&std::isfinite(one.headYaw_));
    eyes_reset(0);avatar_state state;avatar_pose p;eyes_pose legacy={};avatar_reset(&state,0);
    unsigned changed=0,total=0;
    for(unsigned now=0;now<1200;now+=17){avatar_step(&state,now,&legacy,{},&p);bool dirty=false;for(auto r:p.damage)dirty|=r.w&&r.h;changed+=dirty;total++;}
    assert(changed<total);
    action a={};a.kind=ACTION_DRAG;eyes_begin(&a,1250);
    avatar_step(&state,1250,&legacy,{.8f,0,true},&p);
    assert(!engine.touchActive_&&engine.tiltTargetX_>0);
    for(unsigned now=1267;now<=1460;now+=17)avatar_step(&state,now,&legacy,{.8f,0,true},&p);
    float right=engine.interactionX_;
    for(unsigned now=1477;now<=1700;now+=17)avatar_step(&state,now,&legacy,{-.8f,0,true},&p);
    assert(engine.interactionX_<right-40&&!engine.touchActive_);
    eyes_end(true,1700);
    avatar_touch(true,250,240,1800);avatar_step(&state,1800,&legacy,{},&p);assert(engine.touchActive_);
    avatar_touch(false,250,240,1880);avatar_step(&state,1880,&legacy,{},&p);
    avatar_reset(&state,UINT32_MAX-100);
    avatar_step(&state,UINT32_MAX-50,&legacy,{.5f,0,true},&p);avatar_step(&state,50,&legacy,{.5f,0,true},&p);
    assert(std::isfinite(engine.interactionX_));
    printf("Real-time spring, live tilt during MCP, actual finger gaze, wrap and unchanged-frame skipping: passed (%u/%u idle frames changed)\n",changed,total);
}

// SPDX-License-Identifier: AGPL-3.0-or-later
// Display-only input choreography; no input execution or model-state inference.
#pragma once
#include "eyes.h"
#include <cstdint>
enum class AvatarActivityKind:uint8_t {None,Tap,DoubleTap,Hold,Drag,Scroll,Type,Key,Interrupted};
struct AvatarActivityProp {
    AvatarActivityKind kind=AvatarActivityKind::None;
    uint32_t age=0;
    float amount=0,px=233,py=355;
    int vertical=0,horizontal=0;
    unsigned trail_count=0;
    float trail[6][2]{};
};
struct AvatarActivityFrame {
    AvatarActivityKind kind=AvatarActivityKind::None;
    float x=0,y=0,scale=1,lean=0;
    AvatarActivityProp prop{},previous{};
};
class AvatarActivity {
public:
    void reset(uint32_t now);
    void begin(int kind,uint32_t now);
    AvatarActivityFrame sample(uint32_t now,const eyes_event *event,bool local_touch);
private:
    AvatarActivityKind kind_=AvatarActivityKind::None;
    AvatarActivityProp prop_{};
    uint32_t start_=0,ended_=0,last_=0,last_trail_=0;
    bool running_=false;
    float values_[4]{0,0,1,0},velocity_[4]{};
};
// Append bounded native primitives for a single visual layer.
void avatar_activity_draw(const AvatarActivityProp &prop);

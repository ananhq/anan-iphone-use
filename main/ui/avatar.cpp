// SPDX-License-Identifier: AGPL-3.0-or-later
// Native input adapter, 2026-10-15. Gesture mapping adapted from upstream
// src/main.cpp at 204963257cb4dc2f3d7501eff900897bac55ef82; see vendor/stopwatch_avatar/ORIGIN.md.
#include "avatar.h"
#include "avatar_native.hpp"
#include "vendor/stopwatch_avatar/avatar_engine.h"
#include <algorithm>
#include <atomic>
#include <cmath>
namespace {
AvatarEngine engine;
AvatarActivity activity;
std::atomic<unsigned> touch_seq{0},touch_xy{0},touch_origin{0},touch_start{0},touch_end{0},touch_id{0};
std::atomic<bool> touching{false};
struct Contact {unsigned xy,origin,start,end,id;bool down;};
unsigned seen_touch,seen_event,last_click;int last_kind=-1;
bool gesture,committed,looping,have_event;int axis,start_x,start_y;unsigned press_time;
bool snapshot(Contact &c){
    unsigned seq=touch_seq.load();if(seq&1)return false;
    c={touch_xy.load(),touch_origin.load(),touch_start.load(),touch_end.load(),touch_id.load(),touching.load()};
    return seq==touch_seq.load();
}
ExpressionId input_expression(int kind){
    switch(kind){
        case ACTION_TAP:return ExpressionId::Happy;
        case ACTION_DOUBLE_TAP:return ExpressionId::Surprised;
        case ACTION_LONG_PRESS:return ExpressionId::Angry;
        case ACTION_DRAG:case ACTION_DRAG_HOLD:case ACTION_PATH:return ExpressionId::Curious;
        case ACTION_TYPE:return ExpressionId::Excited;
        case ACTION_SCROLL:return ExpressionId::Thinking;
        case ACTION_KEY:case ACTION_KEYBOARD:return ExpressionId::Surprised;
        case ACTION_CONSUMER:return ExpressionId::Excited;
        default:return ExpressionId::Confused;
    }
}
bool continuous(int k){return k==ACTION_DRAG||k==ACTION_DRAG_HOLD||k==ACTION_PATH||k==ACTION_SCROLL||k==ACTION_TYPE;}
bool pointer_kind(int k){return k==ACTION_TAP||k==ACTION_LONG_PRESS||k==ACTION_DOUBLE_TAP||k==ACTION_DRAG||k==ACTION_DRAG_HOLD||k==ACTION_PATH;}
void reset_gesture(){gesture=committed=false;axis=0;engine.releaseTouch();engine.releaseSwipe();}
void touch_update(const Contact &c,unsigned now){
    int x=c.xy&65535,y=c.xy>>16;
    if(c.id!=seen_touch){
        seen_touch=c.id;reset_gesture();gesture=true;press_time=c.start;start_x=c.origin&65535;start_y=c.origin>>16;
    }
    if(!gesture)return;
    int dx=x-start_x,dy=y-start_y;
    if(!axis&&std::max(std::abs(dx),std::abs(dy))>=12)axis=std::abs(dx)>=std::abs(dy)?1:2;
    if(c.down){
        if(axis==1){
            engine.releaseTouch();int direction=dx>0?-1:1;
            engine.setSwipeOffset(dx,0,committed?0:direction);
            if(!committed&&std::abs(dx)>=52){engine.commitSwipe(direction,now,160);committed=true;}
        }else if(axis==2){
            engine.releaseTouch();engine.setSwipeOffset(0,dy,0);
            if(!committed&&std::abs(dy)>=52){engine.show(dy>0?ExpressionId::Sleepy:ExpressionId::Surprised,now,true,160);committed=true;}
        }else if(!committed&&now-press_time>=650){
            engine.releaseTouch();engine.show(ExpressionId::Angry,now);committed=true;last_click=0;
        }else if(!committed)engine.setTouchTarget(x,y);
    }else {
        if(!committed&&!axis){
            bool hold=(uint32_t)(c.end-c.start)>=650;
            bool twice=!hold&&last_click&&(uint32_t)(c.end-last_click)<=300;
            engine.show(hold?ExpressionId::Angry:twice?ExpressionId::Surprised:ExpressionId::Happy,now);
            last_click=twice||hold?0:c.end;
        }
        else last_click=0;
        reset_gesture();
    }
}
}
extern "C" void avatar_touch(bool down,uint16_t x,uint16_t y,uint32_t now){
    // Single touch-task writer. No input calls, allocation, drawing or waiting.
    touch_seq.fetch_add(1);
    touch_xy.store((uint32_t)y<<16|x);
    bool previous=touching.exchange(down);
    if(down&&!previous){touch_origin.store((uint32_t)y<<16|x);touch_start.store(now);touch_id.fetch_add(1);}
    if(!down&&previous)touch_end.store(now);
    touch_seq.fetch_add(1);
}
extern "C" void avatar_reset(avatar_state *state,uint32_t now){
    *state={now,now};engine=AvatarEngine{};avatar_native_clock(now);engine.begin();
    seen_touch=touch_id.load();eyes_event event{};if(eyes_event_snapshot(&event)){seen_event=event.id;last_kind=event.kind;}
    last_click=0;looping=have_event=false;reset_gesture();activity.reset(now);
    avatar_native_set_activity(AvatarActivityFrame{});
}
extern "C" void avatar_step(avatar_state *state,uint32_t now,const eyes_pose *,tilt_gaze tilt,avatar_pose *out){
    avatar_native_clock(now);state->last=now;
    eyes_event event{};bool busy=false;
    bool event_valid=eyes_event_snapshot(&event);
    if(event_valid){
        bool changed=event.kind>=0&&(event.id!=seen_event||(have_event&&event.kind!=last_kind));
        if(changed){
            seen_event=event.id;last_kind=event.kind;have_event=true;reset_gesture();looping=continuous(event.kind);
            activity.begin(event.kind,now);
            ExpressionId expression=input_expression(event.kind);
            if(looping)engine.play(expression,now,AvatarEngine::PlaybackMode::Loop,false);
            else engine.show(expression,now,true);
        }
        busy=have_event&&event.kind>=0&&event.running;
        if(!gesture)engine.releaseTouch();
    }
    Contact contact{};
    bool contact_valid=snapshot(contact);
    if(contact_valid){
        if(busy){seen_touch=contact.id;last_click=0;}
        else touch_update(contact,now);
    }
    AvatarActivityFrame scene=activity.sample(now,have_event&&event_valid?&event:nullptr,contact_valid&&contact.down&&!busy);
    if(looping&&scene.kind==AvatarActivityKind::None){engine.show(ExpressionId::Idle,now);looping=false;}
    float lead_x=0,lead_y=0;
    if(event_valid&&have_event&&scene.kind!=AvatarActivityKind::None){
        if(pointer_kind(event.kind)){
            lead_x=((event.pointer&65535)/32767.5f-1)*.35f*scene.prop.amount;
            lead_y=((event.pointer>>16)/32767.5f-1)*.35f*scene.prop.amount;
        }else if(event.kind==ACTION_SCROLL){
            lead_x=(event.horizontal<0?-.3f:event.horizontal>0?.3f:0)*scene.prop.amount;
            lead_y=(event.vertical<0?.3f:event.vertical>0?-.3f:0)*scene.prop.amount;
        }
    }
    engine.setTiltTarget(tilt.ready?tilt.x:0,tilt.ready?tilt.y:0,lead_x,lead_y);
    avatar_native_set_activity(scene);
    engine.setShakeTarget(scene.lean,0,std::min(1.0f,std::abs(scene.lean)));
    engine.update(now);*out=avatar_native_frame();
}
unsigned avatar_native_expression(){return (unsigned)engine.activeExpression();}

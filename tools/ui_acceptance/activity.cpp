// SPDX-License-Identifier: Apache-2.0
// Defined before activity runtime edits. Synthetic actual avatar pipeline only.
#include "avatar.h"
#include "avatar_native.hpp"
#include "avatar_activity.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <set>
static uint16_t full[466*466],partial[466*466],strip[466*16];
static unsigned raster(const avatar_pose &p){
    unsigned hash=2166136261u;
    for(int y=0;y<466;y+=16){avatar_paint_strip(&p,strip,y);int rows=y+16<466?16:466-y;memcpy(full+y*466,strip,rows*466*2);}
    for(auto v:full)hash=(hash^v)*16777619u;
    for(unsigned i=0;i<AVATAR_DAMAGE_REGIONS;i++){
        auto r=avatar_damage_eye(&p,&p,i);assert(r.x>=0&&r.y>=0&&r.x+r.w<=466&&r.y+r.h<=466);
        if(r.w&&r.h)for(int y=r.y;y<r.y+r.h;){
            int rows=std::min(466*16/r.w,r.y+r.h-y);
            avatar_paint_region(&p,strip,r.x,y,r.w,rows);
            for(int row=0;row<rows;row++)memcpy(partial+(y+row)*466+r.x,strip+row*r.w,r.w*2);
            y+=rows;
        }
    }
    assert(!memcmp(full,partial,sizeof(full)));
    assert(!avatar_native_overflow()&&avatar_native_command_count()<=160);
    return hash;
}
static unsigned colored(){unsigned n=0;for(auto v:full){unsigned c=(v>>8)|(v<<8&65535);unsigned r=c>>11,g=c>>5&63,b=c&31;if(std::abs((int)r-(int)b)>3||std::abs((int)g/2-(int)b)>3)n++;}return n;}
int main(){
    std::set<unsigned> signatures;unsigned maximum_commands=0;
    const action_kind kinds[]={ACTION_TAP,ACTION_DOUBLE_TAP,ACTION_LONG_PRESS,ACTION_DRAG,ACTION_PATH,ACTION_SCROLL,ACTION_TYPE,ACTION_KEYBOARD,ACTION_CONSUMER};
    for(auto kind:kinds){
        eyes_reset(0);avatar_state state;avatar_pose p;eyes_pose legacy={};avatar_reset(&state,0);memset(partial,0,sizeof(partial));
        avatar_step(&state,0,&legacy,{},&p);raster(p);assert(colored()==0);
        action a={};a.kind=kind;a.x=14000;a.y=32768;a.vertical=-1;
        bool begun=false,ended=false;unsigned color_samples=0;float last_x=0,last_y=0,last_scale=1;
        for(unsigned now=17;now<3000;now+=17){
            if(!begun){eyes_begin(&a,now);begun=true;}
            if(now>=85&&!ended){eyes_end(true,now);ended=true;}
            if(kind==ACTION_DRAG||kind==ACTION_PATH)eyes_pointer((uint16_t)std::min(65535u,now*100),32768);
            avatar_step(&state,now,&legacy,{},&p);unsigned hash=raster(p);auto s=avatar_native_activity();
            assert(std::isfinite(s.x)&&std::isfinite(s.y)&&std::isfinite(s.scale));
            assert(std::abs(s.x-last_x)<20&&std::abs(s.y-last_y)<20&&std::abs(s.scale-last_scale)<.16f);
            last_x=s.x;last_y=s.y;last_scale=s.scale;
            if(now>150&&now<480&&colored()>120)color_samples++;
            if(now>=340&&now<357)signatures.insert(hash);
            maximum_commands=std::max(maximum_commands,(unsigned)avatar_native_command_count());
            assert(s.previous.amount==0);
            if(now>1300)assert(s.kind==AvatarActivityKind::None&&colored()==0);
        }
        assert(color_samples>=14);
    }
    assert(signatures.size()>=7);
    eyes_reset(0);avatar_state state;avatar_pose p;eyes_pose legacy={};avatar_reset(&state,0);memset(partial,0,sizeof(partial));
    action a={};
    for(unsigned now=0;now<9000;now+=17){
        if(now<1800&&now%170==0){a.kind=kinds[(now/170)%9];eyes_begin(&a,now);eyes_end(true,now+1);}
        if(now==3400)avatar_touch(true,230,230,now);
        if(now==3468)avatar_touch(false,230,230,now);
        avatar_step(&state,now,&legacy,{},&p);raster(p);
        maximum_commands=std::max(maximum_commands,(unsigned)avatar_native_command_count());
    }
    avatar_reset(&state,10000);memset(partial,0,sizeof(partial));
    for(unsigned now=10000;now<31000;now+=17){avatar_step(&state,now,&legacy,{},&p);raster(p);assert(avatar_native_activity().kind==AvatarActivityKind::None&&colored()==0);}
    avatar_reset(&state,22000);avatar_step(&state,22000,&legacy,{},&p);assert(avatar_native_activity().kind==AvatarActivityKind::None);
    printf("Activity colors, shorter lifetimes, no double props, compact/full equivalence and 20s plain idle: passed (max %u commands)\n",maximum_commands);
}

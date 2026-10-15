// SPDX-License-Identifier: AGPL-3.0-or-later
// Native scanline adapter; upstream engine origins: vendor/stopwatch_avatar/ORIGIN.md.
#include "avatar_native.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#ifdef ESP_PLATFORM
#include "esp_random.h"
#include "esp_timer.h"
#endif
namespace {
enum class Kind:uint8_t {Rect,Round,Circle,Triangle,Line};
struct Command {Kind kind;uint16_t color;int x,y,w,h,r;int x1,y1,x2,y2;};
// Shape-specific six-value storage; all geometry arithmetic remains int.
struct CachedCommand {Kind kind;uint16_t color;int16_t values[6];};
struct CachedBounds {int16_t x,y,w,h;};
static_assert(sizeof(CachedCommand)==16&&sizeof(CachedBounds)==8);
constexpr size_t capacity=160;
CachedCommand commands[capacity];size_t count;
CachedCommand previous_commands[capacity];size_t previous_count;
CachedBounds command_rects[capacity];uint8_t previous_groups[capacity];
avatar_pose frame{};uint32_t now_ms;bool overflow,brow;
unsigned damage_count;
bool full_clear;
AvatarActivityFrame activity;
avatar_rect previous_bounds[AVATAR_DAMAGE_REGIONS]{};
bool cache(CachedCommand &out,const Command &c){
    bool endpoints=c.kind==Kind::Triangle||c.kind==Kind::Line;
    int values[]={c.x,c.y,endpoints?c.x1:c.w,endpoints?c.y1:c.h,
                  endpoints?c.x2:c.r,endpoints?c.y2:0};
    for(int value:values)if(value<INT16_MIN||value>INT16_MAX){overflow=true;return false;}
    out.kind=c.kind;out.color=c.color;
    for(unsigned i=0;i<6;i++)out.values[i]=(int16_t)values[i];
    return true;
}
Command decoded(const CachedCommand &c){
    Command out{};out.kind=c.kind;out.color=c.color;out.x=c.values[0];out.y=c.values[1];
    if(c.kind==Kind::Triangle||c.kind==Kind::Line){
        out.x1=c.values[2];out.y1=c.values[3];out.x2=c.values[4];out.y2=c.values[5];
    }else {out.w=c.values[2];out.h=c.values[3];out.r=c.values[4];}
    return out;
}
void push(Command c){
    // Convert once during collection, instead of for every colored row span.
    c.color=(uint16_t)((c.color>>8)|(c.color<<8));
    if(count<capacity){if(cache(commands[count],c))count++;}else overflow=true;
}
bool same(const CachedCommand &a,const CachedCommand &b){
    if(a.kind!=b.kind||a.color!=b.color)return false;
    for(unsigned i=0;i<6;i++)if(a.values[i]!=b.values[i])return false;
    return true;
}
avatar_rect clipped(int x,int y,int w,int h){
    int end=std::clamp(x+w,0,466),bottom=std::clamp(y+h,0,466);
    x=std::clamp(x,0,466)&~1;y=std::clamp(y,0,466);end=(end+1)&~1;
    return {x,y,std::max(0,end-x),std::max(0,bottom-y)};
}
avatar_rect unite(avatar_rect a,avatar_rect b){
    if(!a.w||!a.h)return b;
    if(!b.w||!b.h)return a;
    int x=std::min(a.x,b.x),y=std::min(a.y,b.y);
    return {x,y,std::max(a.x+a.w,b.x+b.w)-x,std::max(a.y+a.h,b.y+b.h)-y};
}
int transformed(int value,float shift){return (int)std::lround(233+(value-233)*activity.scale+shift);}
void transform(Command &c){
    if(activity.scale==1&&activity.x==0&&activity.y==0)return;
    if(c.kind==Kind::Triangle||c.kind==Kind::Line){
        c.x=transformed(c.x,activity.x);c.y=transformed(c.y,activity.y);
        c.x1=transformed(c.x1,activity.x);c.y1=transformed(c.y1,activity.y);
        c.x2=transformed(c.x2,activity.x);c.y2=transformed(c.y2,activity.y);
    }else if(c.kind==Kind::Circle){
        int x=transformed(c.x+c.r,activity.x),y=transformed(c.y+c.r,activity.y);
        c.r=std::max(1,(int)std::lround(c.r*activity.scale));c.x=x-c.r;c.y=y-c.r;c.w=c.h=2*c.r+1;
    }else {
        int right=transformed(c.x+c.w,activity.x),bottom=transformed(c.y+c.h,activity.y);
        c.x=transformed(c.x,activity.x);c.y=transformed(c.y,activity.y);
        c.w=std::max(1,right-c.x);c.h=std::max(1,bottom-c.y);
        c.r=std::clamp((int)std::lround(c.r*activity.scale),0,std::min(c.w,c.h)/2);
    }
}
avatar_rect command_bounds(const Command &c){
    if(c.kind==Kind::Triangle){
        int x=std::min({c.x,c.x1,c.x2}),y=std::min({c.y,c.y1,c.y2});
        return clipped(x,y,std::max({c.x,c.x1,c.x2})-x+1,std::max({c.y,c.y1,c.y2})-y+1);
    }
    if(c.kind==Kind::Line)return clipped(std::min(c.x,c.x1),std::min(c.y,c.y1),std::abs(c.x1-c.x)+1,std::abs(c.y1-c.y)+1);
    return clipped(c.x,c.y,c.w,c.h);
}
void span(uint16_t *pixels,int clip_x,int top,int width,int rows,int y,int left,int right,uint16_t color){
    if(y<top||y>=top+rows||y>=466)return;
    left=std::max(clip_x,left);right=std::min(clip_x+width-1,right);if(left>right)return;
    left-=clip_x;right-=clip_x;
    uint16_t *row=pixels+(y-top)*width;
    if(color==0||color==0xffff)std::memset(row+left,color?255:0,(right-left+1)*2);
    else std::fill(row+left,row+right+1,color);
}
void triangle_row(const Command &c,int y,int &left,int &right){
    int xs[]={c.x,c.x1,c.x2},ys[]={c.y,c.y1,c.y2};float lo=1e6f,hi=-1e6f;
    for(unsigned i=0;i<3;i++){
        unsigned j=(i+1)%3;int a=ys[i],b=ys[j];
        if(y<std::min(a,b)||y>std::max(a,b))continue;
        if(a==b){lo=std::min(lo,(float)std::min(xs[i],xs[j]));hi=std::max(hi,(float)std::max(xs[i],xs[j]));}
        else {float x=xs[i]+(float)(y-a)*(xs[j]-xs[i])/(b-a);lo=std::min(lo,x);hi=std::max(hi,x);}
    }
    left=(int)std::ceil(lo);right=(int)std::floor(hi);
}
}
AvatarHardware M5;AvatarQuietLog Serial;
void avatar_native_clock(uint32_t now){now_ms=now;}
uint32_t millis(){return now_ms;}
uint32_t micros(){
#ifdef ESP_PLATFORM
    return (uint32_t)esp_timer_get_time();
#else
    return now_ms*1000;
#endif
}
long random(long minimum,long maximum){
    if(maximum<=minimum)return minimum;
#ifdef ESP_PLATFORM
    uint32_t value=esp_random();
#else
    static uint32_t value=0x739a124b;value^=value<<13;value^=value>>17;value^=value<<5;
#endif
    return minimum+value%(maximum-minimum);
}
void AvatarDisplay::startWrite(){count=0;damage_count=0;overflow=false;brow=false;full_clear=false;for(auto &r:frame.damage)r={};}
void AvatarDisplay::endWrite(){
    if(overflow)return;
    avatar_rect bounds[AVATAR_DAMAGE_REGIONS]{};
    for(size_t i=0;i<count;i++){
        Command c=decoded(commands[i]);transform(c);if(!cache(commands[i],c))return;
    }
    size_t props_begin=count;
    avatar_activity_draw(activity.prop);
    if(overflow)return;
    bool changed[AVATAR_DAMAGE_REGIONS]{};
    for(size_t i=0;i<count;i++){
        avatar_rect r=command_bounds(decoded(commands[i]));
        // command_bounds clips these values to the physical 466px screen.
        command_rects[i]={(int16_t)r.x,(int16_t)r.y,(int16_t)r.w,(int16_t)r.h};
        unsigned group=i>=props_begin?2:(r.x+r.w/2<233?0:1);
        bounds[group]=unite(bounds[group],r);
        if(i>=previous_count){changed[group]=true;}
        else if(previous_groups[i]!=group||!same(previous_commands[i],commands[i])){
            changed[group]=true;changed[previous_groups[i]]=true;
        }
        previous_commands[i]=commands[i];previous_groups[i]=(uint8_t)group;
    }
    for(size_t i=count;i<previous_count;i++)changed[previous_groups[i]]=true;
    previous_count=count;
    for(unsigned i=0;i<AVATAR_DAMAGE_REGIONS;i++){
        frame.damage[i]=changed[i]?unite(previous_bounds[i],bounds[i]):avatar_rect{};
        previous_bounds[i]=bounds[i];
    }
    if(full_clear){frame.damage[0]={0,0,466,466};frame.damage[1]=frame.damage[2]={};}
    if(++frame.frame==0)++frame.frame;
}
void AvatarDisplay::fillScreen(uint16_t color){(void)color;full_clear=true;damage_count=2;}
void AvatarDisplay::clearDamage(int x,int y,int w,int h){
    // Actual transformed primitive bounds are tracked at endWrite. The engine's
    // untransformed rectangles cannot clear moved faces or independent props.
    (void)x;(void)y;(void)w;(void)h;
    if(damage_count<2)damage_count++;else overflow=true;
}
void AvatarDisplay::fillRect(int x,int y,int w,int h,uint16_t color){if(w>0&&h>0)push({Kind::Rect,color,x,y,w,h,0,0,0,0,0});}
void AvatarDisplay::fillRoundRect(int x,int y,int w,int h,int r,uint16_t color){if(w>0&&h>0)push({Kind::Round,color,x,y,w,h,std::clamp(r,0,std::min(w,h)/2),0,0,0,0});}
void AvatarDisplay::fillCircle(int x,int y,int r,uint16_t color){if(r>0)push({Kind::Circle,color,x-r,y-r,2*r+1,2*r+1,r,0,0,0,0});}
void AvatarDisplay::fillTriangle(int x0,int y0,int x1,int y1,int x2,int y2,uint16_t color){push({Kind::Triangle,color,x0,y0,0,0,0,x1,y1,x2,y2});}
void AvatarDisplay::drawLine(int x0,int y0,int x1,int y1,uint16_t color){push({Kind::Line,color,x0,y0,0,0,0,x1,y1,0,0});}
void AvatarDisplay::drawWideLine(float x0,float y0,float x1,float y1,float radius,uint16_t color){
    brow=true;float dx=x1-x0,dy=y1-y0,length=std::hypot(dx,dy);
    if(length>0){float nx=-dy/length*radius,ny=dx/length*radius;
        fillTriangle(std::lround(x0+nx),std::lround(y0+ny),std::lround(x0-nx),std::lround(y0-ny),std::lround(x1+nx),std::lround(y1+ny),color);
        fillTriangle(std::lround(x0-nx),std::lround(y0-ny),std::lround(x1-nx),std::lround(y1-ny),std::lround(x1+nx),std::lround(y1+ny),color);
    }
    fillCircle(std::lround(x0),std::lround(y0),std::max(1,(int)std::lround(radius)),color);
    fillCircle(std::lround(x1),std::lround(y1),std::max(1,(int)std::lround(radius)),color);
}
avatar_pose avatar_native_frame(){return frame;}
bool avatar_native_overflow(){return overflow;}
size_t avatar_native_command_count(){return count;}
bool avatar_native_has_brow(){return brow;}
void avatar_native_set_activity(const AvatarActivityFrame &next){activity=next;}
AvatarActivityFrame avatar_native_activity(){return activity;}
extern "C" bool avatar_frame_ready(const avatar_pose *p){return !overflow&&p->frame&&p->frame==frame.frame;}
extern "C" avatar_rect avatar_damage_eye(const avatar_pose *,const avatar_pose *next,unsigned eye){return eye<AVATAR_DAMAGE_REGIONS?next->damage[eye]:avatar_rect{};}
extern "C" void avatar_paint_strip(const avatar_pose *p,uint16_t *pixels,int top){
    avatar_paint_region(p,pixels,0,top,466,16);
}
extern "C" void avatar_paint_region(const avatar_pose *p,uint16_t *pixels,int clip_x,int top,int width,int rows){
    if(width<=0||rows<=0)return;
    std::memset(pixels,0,(size_t)width*rows*2);
    if(!avatar_frame_ready(p)||clip_x<0||clip_x+width>466||top<0)return;
    for(size_t i=0;i<count;i++){
        const Command c=decoded(commands[i]);const CachedBounds &r=command_rects[i];
        if(r.x+r.w<=clip_x||r.x>=clip_x+width||r.y+r.h<=top||r.y>=top+rows)continue;
        if(c.kind==Kind::Line){
            int x=c.x,y=c.y,dx=std::abs(c.x1-x),sx=x<c.x1?1:-1,dy=-std::abs(c.y1-y),sy=y<c.y1?1:-1,err=dx+dy;
            for(;;){span(pixels,clip_x,top,width,rows,y,x,x,c.color);if(x==c.x1&&y==c.y1)break;int e=2*err;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}
            continue;
        }
        int first=c.y,last=c.y+c.h-1;
        if(c.kind==Kind::Triangle){first=std::min({c.y,c.y1,c.y2});last=std::max({c.y,c.y1,c.y2});}
        first=std::max(first,top);last=std::min({last,top+rows-1,465});
        for(int y=first;y<=last;y++){
            int left=c.x,right=c.x+c.w-1;
            if(c.kind==Kind::Circle){int d=y-c.y-c.r,disc=c.r*c.r-d*d;int extent=(int)std::sqrt((float)std::max(0,disc));left=c.x+c.r-extent;right=c.x+c.r+extent;}
            else if(c.kind==Kind::Round&&c.r){
                int offset=y<c.y+c.r?c.y+c.r-y:y>c.y+c.h-1-c.r?y-(c.y+c.h-1-c.r):0;
                int extent=(int)std::sqrt((float)(c.r*c.r-offset*offset));left+=c.r-extent;right-=c.r-extent;
            }else if(c.kind==Kind::Triangle)triangle_row(c,y,left,right);
            span(pixels,clip_x,top,width,rows,y,left,right,c.color);
        }
    }
}

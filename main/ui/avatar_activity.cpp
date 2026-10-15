// SPDX-License-Identifier: AGPL-3.0-or-later
#include "avatar_activity.hpp"
#include "avatar_native.hpp"
#include <algorithm>
#include <cmath>
namespace {
constexpr float pi=3.14159265359f;
float smooth(float x){x=std::clamp(x,0.0f,1.0f);return x*x*(3-2*x);}
void follow(float &x,float &v,float target,float dt){
    constexpr float rate=25;
    float offset=x-target,step=(v+rate*offset)*dt,decay=std::exp(-rate*dt);
    x=target+(offset+step)*decay;v=(v-rate*step)*decay;
}
AvatarActivityKind mapped(int kind){
    switch(kind){
    case ACTION_TAP:return AvatarActivityKind::Tap;
    case ACTION_DOUBLE_TAP:return AvatarActivityKind::DoubleTap;
    case ACTION_LONG_PRESS:return AvatarActivityKind::Hold;
    case ACTION_DRAG:case ACTION_DRAG_HOLD:case ACTION_PATH:return AvatarActivityKind::Drag;
    case ACTION_SCROLL:return AvatarActivityKind::Scroll;
    case ACTION_TYPE:return AvatarActivityKind::Type;
    case ACTION_KEY:case ACTION_KEYBOARD:case ACTION_CONSUMER:return AvatarActivityKind::Key;
    default:return AvatarActivityKind::Interrupted;
    }
}
uint16_t color(int r,int g,int b,float amount){
    amount=std::clamp(amount,0.0f,1.0f);
    return M5.Display.color565((uint8_t)(r*amount),(uint8_t)(g*amount),(uint8_t)(b*amount));
}
void stroke(int x,int y,int endx,int endy,float width,uint16_t c){M5.Display.drawWideLine(x,y,endx,endy,width,c);}
void ring(int x,int y,int radius,int width,uint16_t c){M5.Display.fillCircle(x,y,radius,c);M5.Display.fillCircle(x,y,std::max(1,radius-width),0);}
void cursor(int x,int y,float down,uint16_t c){
    y+=(int)std::lround(down*10);
    // Rounded diagonal stem and broad arrow silhouette fit the white-eye style.
    M5.Display.fillTriangle(x,y,x+5,y+48,x+16,y+34,c);
    M5.Display.fillTriangle(x,y,x+16,y+34,x+42,y+31,c);
    stroke(x+14,y+29,x+27,y+49,4,c);
}
float bounce(unsigned age){return std::sin(std::min(1.0f,age/440.0f)*pi)*std::exp(-(float)age*.0015f);}
}
void AvatarActivity::reset(uint32_t now){
    *this=AvatarActivity{};start_=ended_=last_=now;
}
void AvatarActivity::begin(int kind,uint32_t now){
    kind_=mapped(kind);start_=last_trail_=now;running_=true;
    prop_=AvatarActivityProp{};prop_.kind=kind_;
}
AvatarActivityFrame AvatarActivity::sample(uint32_t now,const eyes_event *event,bool local_touch){
    (void)local_touch;
    float dt=(uint32_t)(now-last_)*.001f;last_=now;
    bool action=kind_!=AvatarActivityKind::None;
    if(action&&event){
        running_=event->running;ended_=event->ended;
        prop_.px=190+((event->pointer&65535)/65535.0f)*86;
        prop_.py=325+((event->pointer>>16)/65535.0f)*30;
        prop_.vertical=event->vertical;prop_.horizontal=event->horizontal;
    }
    unsigned age=now-start_;float amount=0;
    if(action){
        amount=.3f+.7f*smooth(age/150.0f);
        if(!running_&&age>=480&&(int32_t)(now-ended_)>=120){
            uint32_t fading=std::min<uint32_t>((uint32_t)age-480,now-ended_-120);
            amount*=1-smooth(fading/140.0f);
            if(fading>=140){kind_=AvatarActivityKind::None;amount=0;}
        }
    }
    // Secondary props update at about 30fps; eye/IMU updates keep their cadence.
    prop_.kind=kind_;prop_.age=age/33*33;prop_.amount=amount;
    if(kind_==AvatarActivityKind::Drag&&event&&(prop_.trail_count==0||(uint32_t)(now-last_trail_)>=55)){
        last_trail_=now;
        unsigned count=std::min(5u,prop_.trail_count);
        for(unsigned i=count;i>0;i--){prop_.trail[i][0]=prop_.trail[i-1][0];prop_.trail[i][1]=prop_.trail[i-1][1];}
        prop_.trail[0][0]=prop_.px;prop_.trail[0][1]=prop_.py;prop_.trail_count=count+1;
    }
    float x=0,y=0,scale=1,lean=0,t=age*.001f;
    switch(kind_){
    case AvatarActivityKind::Tap:case AvatarActivityKind::DoubleTap:case AvatarActivityKind::Key:{
        float b=bounce(age);
        if(kind_==AvatarActivityKind::DoubleTap&&age>230)b+=bounce(age-230)*.7f;
        y=-18+32*b;scale=1.05f+.22f*b;break;
    }
    case AvatarActivityKind::Hold:y=-14+6*std::sin(t*5);scale=.96f;lean=.28f*std::sin(t*3);break;
    case AvatarActivityKind::Drag:
        x=(prop_.px-233)*.55f;y=-22+(prop_.py-340)*.7f;scale=1.06f;lean=(prop_.px-233)/50;break;
    case AvatarActivityKind::Scroll:
        x=(prop_.horizontal<0?-12:prop_.horizontal>0?12:0);y=-22+20*std::sin(t*8)*(prop_.vertical<0?1:-1);scale=1.04f;lean=.18f*std::sin(t*8);break;
    case AvatarActivityKind::Type:x=10*std::sin(t*12);y=-25+14*std::sin(t*22);scale=1.08f+.07f*std::sin(t*11);lean=.3f*std::sin(t*12);break;
    case AvatarActivityKind::Interrupted:x=8*std::sin(t*7);y=-12;scale=.98f;lean=-.6f;break;
    default:break;
    }
    float desired[]={x*amount,y*amount,1+(scale-1)*amount,lean*amount};
    for(unsigned i=0;i<4;i++)follow(values_[i],velocity_[i],desired[i],dt);
    AvatarActivityFrame result;result.kind=kind_;result.x=values_[0];result.y=values_[1];result.scale=values_[2];result.lean=values_[3];result.prop=prop_;
    return result;
}
void avatar_activity_draw(const AvatarActivityProp &p){
    if(p.kind==AvatarActivityKind::None||p.amount<.005f)return;
    float a=p.amount,t=p.age*.001f;
    uint16_t cyan=color(83,218,239,a),blue=color(105,166,255,a),purple=color(182,151,255,a),amber=color(255,195,98,a);
    int entrance=(int)std::lround((1-a)*16);
    switch(p.kind){
    case AvatarActivityKind::Tap:case AvatarActivityKind::DoubleTap:{
        int x=226,y=337+entrance;
        for(unsigned n=0;n<(p.kind==AvatarActivityKind::DoubleTap?2u:1u);n++){
            int age=(int)p.age-(int)n*230;
            if(age>=0&&age<650){float phase=age/650.0f;ring(x,y,10+(int)(34*phase),3,color(83,218,239,a*(1-phase)));}
        }
        cursor(x,y,bounce(p.age),cyan);break;
    }
    case AvatarActivityKind::Hold:{
        int y=350+entrance;ring(233,y,40+(int)(3*std::sin(t*5)),3,amber);
        M5.Display.fillCircle(233,y,23,amber);cursor(229,y-19,.8f,color(248,248,248,a));break;
    }
    case AvatarActivityKind::Drag:{
        for(unsigned n=p.trail_count;n>1;n--){
            unsigned i=n-1;stroke((int)p.trail[i][0],(int)p.trail[i][1],(int)p.trail[i-1][0],(int)p.trail[i-1][1],2.5f,color(83,218,239,a*(1-i/7.0f)));
        }
        ring((int)p.px,(int)p.py+entrance,7,2,color(83,218,239,a*.55f));
        cursor((int)p.px,(int)p.py+entrance,.2f,cyan);break;
    }
    case AvatarActivityKind::Scroll:{
        int dir=p.vertical<0?1:-1;bool horizontal=p.horizontal&&!p.vertical;
        float cycle=std::fmod(t*1.5f,1.0f);
        for(int i=-1;i<=1;i++){
            float offset=(i+cycle)*29;float fade=std::max(0.0f,1-std::abs(offset)/62)*a;
            int x=horizontal?190+(int)(offset*(p.horizontal<0?-1:1)):190;
            int y=horizontal?348+entrance:348+(int)(offset*dir)+entrance;
            uint16_t c=color(105,166,255,fade);
            M5.Display.fillRoundRect(x,y,86,20,7,c);
            M5.Display.fillRoundRect(x+4,y+4,78,12,4,0);
            M5.Display.fillRoundRect(x+13,y+8,39,4,2,c);
        }
        break;
    }
    case AvatarActivityKind::Type:{
        int x=158,y=326+entrance;
        M5.Display.fillRoundRect(x,y,150,68,14,color(37,48,65,a));
        M5.Display.fillRoundRect(x+3,y+3,144,62,11,color(17,23,32,a));
        unsigned active=(p.age/95)%12;float jump=std::sin((p.age%95)/95.0f*pi);
        for(unsigned i=0;i<12;i++){
            int dy=i==active?(int)(10*jump):0;
            uint16_t c=i==active?purple:color(118,140,169,a*.8f);
            M5.Display.fillRoundRect(x+11+(i%6)*22,y+12+(i/6)*21-dy,18,14,4,c);
        }
        M5.Display.fillRoundRect(x+48,y+55,54,5,2,blue);break;
    }
    case AvatarActivityKind::Key:{
        int down=(int)(11*bounce(p.age)),x=192,y=325+entrance+down;
        M5.Display.fillRoundRect(x,y+9,82,65,15,color(75,56,105,a));
        M5.Display.fillRoundRect(x,y,82,62,15,purple);
        uint16_t ink=color(38,27,61,a);
        stroke(x+55,y+18,x+55,y+34,2.5f,ink);stroke(x+55,y+34,x+28,y+34,2.5f,ink);
        stroke(x+28,y+34,x+36,y+26,2.5f,ink);stroke(x+28,y+34,x+36,y+42,2.5f,ink);break;
    }
    case AvatarActivityKind::Interrupted:{
        int x=235,y=333+entrance;
        stroke(x-15,y,x-8,y-10,4,amber);stroke(x-8,y-10,x+9,y-10,4,amber);
        stroke(x+9,y-10,x+17,y,4,amber);stroke(x+17,y,x+14,y+10,4,amber);
        stroke(x+14,y+10,x,y+21,4,amber);stroke(x,y+21,x,y+27,4,amber);
        M5.Display.fillCircle(x,y+43,5,amber);break;
    }
    default:break;
    }
}

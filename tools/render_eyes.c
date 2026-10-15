// SPDX-License-Identifier: Apache-2.0
// Synthetic document preview from the firmware sampler/painter; no device I/O.
#include "eyes.h"
#include "avatar.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv) {
    if(argc!=3)return 2;
    int mode=atoi(argv[1]);unsigned now=(unsigned)strtoul(argv[2],NULL,10);
    eyes_reset(0);avatar_state state;avatar_pose pose;avatar_reset(&state,0);
    action a={.kind=mode==1?ACTION_TAP:mode==2?ACTION_DRAG:mode==3?ACTION_TYPE:
        mode==4?ACTION_SCROLL:ACTION_KEYBOARD,.x=32768,.y=28000,.vertical=-1};
    if(mode>0&&mode<=6)eyes_begin(&a,0);
    bool ended=false;int phase=-1;
    for(unsigned t=0;;){
        if(mode==10){
            static const unsigned starts[]={6500,7800,9200,10600,11800};
            static const unsigned durations[]={70,1000,900,300,30};
            static const action_kind kinds[]={ACTION_TAP,ACTION_DRAG,ACTION_TYPE,ACTION_SCROLL,ACTION_KEYBOARD};
            int next=-1;for(int i=0;i<5;i++)if(t>=starts[i])next=i;
            if(next>=0&&next!=phase){a.kind=kinds[next];eyes_begin(&a,starts[next]);phase=next;ended=false;}
            if(phase>=0&&!ended&&t>=starts[phase]+durations[phase]){eyes_end(true,starts[phase]+durations[phase]);ended=true;}
            if(phase==1){unsigned progress=t-starts[1];if(progress>1000)progress=1000;eyes_pointer((uint16_t)(65535*progress/1000),32768);}
            if(t>=4500)avatar_touch(t<5700,(uint16_t)(233+150*sinf((t-4500)*.0018f)),(uint16_t)(233-90*sinf((t-4500)*.002f)),t);
        }
        if(mode==9){
            int next=(int)(t/650)%4;
            if(next!=phase){a.kind=next==0?ACTION_TAP:next==1?ACTION_DRAG:next==2?ACTION_TYPE:ACTION_KEYBOARD;eyes_begin(&a,t);phase=next;ended=false;}
            if(!ended&&t%650>=100){eyes_end(true,t);ended=true;}
        }else if(mode>0&&mode<=6){
            unsigned duration=mode==1?70:1000;
            if(!ended&&t>=duration){eyes_end(mode!=6,duration);ended=true;}
        }
        if(mode==2){unsigned progress=t<1000?t:1000;eyes_pointer((uint16_t)(65535*progress/1000),(uint16_t)(65535*progress/1000));}
        if(mode==8)avatar_touch(t>=100&&t<1100,(uint16_t)(233+160*sinf(t*.002f)),(uint16_t)(233-100*sinf(t*.003f)),t);
        eyes_pose target;eyes_sample(t,&target);
        tilt_gaze gaze=mode==7?(tilt_gaze){sinf(t*.0015f),.5f*sinf(t*.002f),true}:(tilt_gaze){0};
        if(mode==10&&t>=1200&&t<4200){float wave=sinf((t-1200)*6.2831853f/3000);gaze=(tilt_gaze){wave,-.6f*wave,true};}
        avatar_step(&state,t,&target,gaze,&pose);
        if(t==now)break;
        t=now-t<33?now:t+33;
    }
    printf("P6\n466 466\n255\n");
    uint16_t pixels[466*16];unsigned char rgb[466*16*3];
    for(int y=0;y<466;y+=16) {
        avatar_paint_strip(&pose,pixels,y);
        int count=(y+16<=466?16:466-y)*466;
        for(int i=0;i<count;i++) {
            unsigned color=(pixels[i]>>8)|(pixels[i]<<8&65535);
            rgb[i*3]=(color>>11)*255/31;
            rgb[i*3+1]=(color>>5&63)*255/63;
            rgb[i*3+2]=(color&31)*255/31;
        }
        fwrite(rgb,3,(size_t)count,stdout);
    }
    return ferror(stdout)?1:0;
}

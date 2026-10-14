#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "hid_input.h"
unsigned fake_ms;
static unsigned cutoff=~0u;
typedef struct { char kind; unsigned time; int buttons,x,y,v,h,key,mods,usage; } event;
static event events[1024];
static unsigned event_count;
static event* record(char k){assert(event_count<1024);event*e=&events[event_count++];memset(e,0,sizeof(*e));e->kind=k;e->time=fake_ms;return e;}
bool bridge_mouse(uint8_t b,uint16_t x,uint16_t y){event*e=record('m');e->buttons=b;e->x=x;e->y=y;return true;}
bool bridge_scroll(uint16_t x,uint16_t y,int8_t v,int8_t h){event*e=record('w');e->x=x;e->y=y;e->v=v;e->h=h;return true;}
bool bridge_key(uint8_t m,uint8_t k){event*e=record('k');e->key=k;e->mods=m;return true;}
bool bridge_consumer(uint16_t u){event*e=record('c');e->usage=u;return true;}
bool bridge_usb_online(void){return true;}
void bridge_release(void){bridge_mouse(0,0,0);bridge_key(0,0);bridge_consumer(0);}
#include "actions.c"
static bool guard(uint32_t epoch,void*ctx){(void)epoch;(void)ctx;return fake_ms<cutoff;}
static action parse(const char*n,const char*json,bool expected){action a;cJSON*j=cJSON_Parse(json);assert(j);bool ok=hid_input_parse(n,j,&a);cJSON_Delete(j);if(ok!=expected){fprintf(stderr,"parse failed: %s %s\n",n,json);abort();}return a;}
static void reset(void){fake_ms=0;cutoff=~0u;event_count=0;current_guard=guard;current_context=NULL;}
static bool run(action*a){reset();bool ok=execute(a);bridge_release();return ok;}
int main(void){
 action merged=parse("tap","{\"x\":0.5,\"y\":0.25}",true);assert(merged.kind==ACTION_TAP);assert(run(&merged));assert(fake_ms==70);
 merged=parse("tap","{\"x\":0.5,\"y\":0.25,\"holdMs\":700}",true);assert(merged.kind==ACTION_LONG_PRESS);assert(run(&merged));assert(events[2].time-events[1].time==700);
 merged=parse("tap","{\"x\":0.5,\"y\":0.25,\"count\":2,\"intervalMs\":150}",true);assert(merged.kind==ACTION_DOUBLE_TAP);assert(run(&merged));assert(events[3].time-events[2].time==150);
 parse("tap","{\"x\":0,\"y\":0,\"count\":2,\"holdMs\":500}",false);
 parse("tap","{\"x\":0,\"y\":0,\"holdMs\":40}",false);
 parse("tap","{\"x\":0,\"y\":0,\"count\":3}",false);
 parse("tap","{\"x\":0,\"y\":0,\"intervalMs\":100}",false);
 merged=parse("drag","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":1}],\"durationMs\":100}",true);assert(merged.kind==ACTION_DRAG);assert(run(&merged));event saved[1024];unsigned saved_count=event_count,saved_ms=fake_ms;memcpy(saved,events,sizeof(events));
 action compat=parse("drag","{\"x\":0,\"y\":0,\"toX\":1,\"toY\":1,\"durationMs\":100}",true);assert(run(&compat));assert(saved_count==event_count&&saved_ms==fake_ms&&!memcmp(saved,events,event_count*sizeof(event)));
 merged=parse("drag","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":1}],\"durationMs\":100,\"holdMs\":500}",true);assert(merged.kind==ACTION_DRAG_HOLD);assert(run(&merged));assert(events[2].time-events[1].time>=500);
 merged=parse("drag","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":0},{\"x\":1,\"y\":1}],\"durationMs\":100}",true);assert(merged.kind==ACTION_PATH);assert(run(&merged));assert(merged.completed_units==2);
 parse("drag","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":1}],\"x\":0,\"durationMs\":100}",false);
 parse("drag","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":1}],\"durationMs\":100,\"holdMs\":-1}",false);
 parse("drag","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":1}],\"durationMs\":100,\"holdMs\":0.5}",false);
 merged=parse("press_key","{\"key\":\"paste\"}",true);assert(merged.kind==ACTION_KEYBOARD&&merged.key==25&&merged.modifiers==8);assert(run(&merged));
 merged=parse("press_key","{\"key\":\"tab\",\"repeat\":2}",true);assert(run(&merged));assert(merged.completed_units==2);
 parse("press_key","{\"key\":\"paste\",\"repeat\":2}",false);
 parse("press_key","{\"key\":\"home\",\"repeat\":2}",false);
 parse("press_key","{\"keycode\":43}",false);
 const char*consumer_names[]={"home","search","toggle_keyboard","globe","volume_up","volume_down","mute","play_pause","next_track","previous_track","brightness_up","brightness_down"};
 for(unsigned i=0;i<sizeof(consumer_names)/sizeof(consumer_names[0]);i++){char canonical[100],legacy[100];snprintf(canonical,sizeof(canonical),"{\"key\":\"%s\"}",consumer_names[i]);snprintf(legacy,sizeof(legacy),"{\"action\":\"%s\"}",consumer_names[i]);merged=parse("press_key",canonical,true);compat=parse("system_key",legacy,true);assert(merged.kind==ACTION_CONSUMER&&merged.consumer==compat.consumer&&merged.press_ms==compat.press_ms);assert(run(&merged));assert(events[1].usage==0);}
 action a=parse("keyboard","{\"key\":\"shift_tab\",\"repeat\":2}",true);assert(run(&a));assert(events[0].key==43&&events[0].mods==2);assert(a.completed_units==2);
 parse("keyboard","{\"key\":\"space\",\"repeat\":2}",false);
 parse("keyboard","{\"key\":\"fn_h\"}",false);
 parse("keyboard","{\"key\":\"tab\",\"pressMs\":19}",false);
 parse("keyboard","{\"key\":\"tab\",\"repeat\":1.5}",false);
 parse("keyboard","{\"key\":\"tab\",\"mystery\":1}",false);
 a=parse("keyboard","{\"key\":\"paste\"}",true);assert(run(&a));assert(events[0].key==25&&events[0].mods==8);
 a=parse("scroll","{\"x\":0.25,\"y\":0.5,\"vertical\":-3,\"horizontal\":2,\"repeat\":2}",true);assert(run(&a));unsigned wheels=0;for(unsigned i=0;i<event_count;i++)if(events[i].kind=='w'){assert(events[i].v==-3&&events[i].h==2);wheels++;}assert(wheels==2&&a.completed_units==2);
 parse("scroll","{\"x\":0,\"y\":0,\"vertical\":0}",false);
 parse("scroll","{\"x\":1.01,\"y\":0,\"vertical\":1}",false);
 a=parse("long_press","{\"x\":0.5,\"y\":0.5,\"durationMs\":700}",true);assert(run(&a));assert(events[1].buttons==1&&events[2].buttons==0);assert(events[2].time-events[1].time>=700);
 a=parse("double_tap","{\"x\":0,\"y\":0,\"intervalMs\":100}",true);assert(run(&a));assert(a.completed_units==2);assert(events[3].buttons==1&&events[3].time-events[2].time>=100);
 a=parse("drag_hold","{\"x\":0,\"y\":0,\"toX\":1,\"toY\":1,\"durationMs\":100,\"holdMs\":500}",true);assert(run(&a));assert(events[2].time-events[1].time>=500);assert(events[2].buttons==1&&events[2].x>0);assert(a.completed_units==1);
 a=parse("draw_path","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":0},{\"x\":1,\"y\":1}],\"durationMs\":100}",true);assert(run(&a));bool corner=false;for(unsigned i=2;i<event_count;i++){if(events[i].buttons==1&&events[i].x==65535&&events[i].y==0)corner=true;}assert(corner&&a.completed_units==2);assert(events[event_count-4].x==65535&&events[event_count-4].y==65535);
 parse("draw_path","{\"points\":[{\"x\":0,\"y\":0}],\"durationMs\":100}",false);
 parse("draw_path","{\"points\":[{\"x\":0,\"y\":0},{\"x\":1,\"y\":1}],\"durationMs\":0}",false);
 a=parse("system_key","{\"action\":\"home\"}",true);assert(run(&a));assert(events[0].usage==0x40&&events[1].usage==0&&a.completed_units==1);
 a=parse("system_key","{\"action\":\"search\"}",true);assert(a.consumer==0x221);
 parse("system_key","{\"action\":\"back\"}",false);
 a=parse("long_press","{\"x\":0,\"y\":0,\"durationMs\":700}",true);reset();cutoff=100;assert(!execute(&a));bridge_release();assert(fake_ms<150&&a.completed_units==0);assert(events[event_count-3].buttons==0);
 a=parse("keyboard","{\"key\":\"tab\",\"repeat\":5}",true);reset();cutoff=60;assert(!execute(&a));bridge_release();assert(a.completed_units==1);
 reset();action old={.kind=ACTION_KEY,.key=43};assert(execute(&old));assert(events[0].key==43&&events[1].key==0&&events[1].time==15&&fake_ms==30);
 reset();old=(action){.kind=ACTION_TAP,.x=100,.y=200};assert(execute(&old));assert(events[0].buttons==0&&events[1].buttons==1&&events[2].buttons==0&&fake_ms==70);
 puts("offline HID acceptance scenarios passed (simulated time/reports; no devices)");return 0;
}

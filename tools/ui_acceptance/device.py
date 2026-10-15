#!/usr/bin/env python3
"""Pre-implementation host scenarios using actual control/touch/board functions.
SDK/time/touch reads are synthetic; never connects to a board or phone.
"""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import re
import subprocess
import tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'main/app/device.c').read_text()
prefix='\n'.join(line for line in source.split('static void buttons(void *unused)')[0].splitlines() if not line.startswith('#include'))
buttons='static void buttons(void *unused)'+source.split('static void buttons(void *unused)')[1].split('static void touch_input(void *unused)')[0]
buttons=buttons.replace('for (;;)','for (unsigned step=0; step<key_count; step++)')
touch=source.split('static void touch_input(void *unused)')[1].split('static void screen(void *unused)')[0]
touch='static void touch_input(void *unused)'+touch.replace('for (;;)','for (unsigned step=0; step<frame_count; step++)')
transition=next(line.strip() for line in source.splitlines() if 'if(previous_ap&&' in line)
board=(root/'main/hardware/board.c').read_text().split('bool board_battery_voltage(')[1]
board='bool board_battery_voltage('+board.split('\nbool board_stop_pressed')[0]
stub=r'''
#include "ui.h"
#include "eyes.h"
#include "avatar.h"
#include "reset_trace.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
typedef int portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(x) ((void)(x))
#define portEXIT_CRITICAL(x) ((void)(x))
#define ESP_OK 0
#define pdMS_TO_TICKS(x) (x)
static uint64_t clock_ms;
static unsigned keys[220],key_count;static uint64_t key_start;
static int gpio_get_level(int pin){unsigned i=(unsigned)(clock_ms-key_start)/5;return !(keys[i]&(pin==2?1u:2u));}
static int64_t esp_timer_get_time(void){return clock_ms*1000;}
static void vTaskDelay(unsigned ms){clock_ms+=ms;}
typedef struct {bool configured,connected,setup_ap,ready;int setup_stage,setup_error;} wireless_state;
static wireless_state network;
static bool ap_requested;
static bool busy,raw_stop,online=true;
static input_mode_t selected;
static unsigned releases,ble_resets,mcp_resets,face_touches;
void avatar_touch(bool down,uint16_t x,uint16_t y,uint32_t now){(void)x;(void)y;(void)now;if(down)face_touches++;}
static bool bridge_input_online(void){return online;}
static bool bridge_usb_online(void){return true;}
static input_mode_t bridge_input_mode(void){return selected;}
static bool bridge_select_input(input_mode_t mode){if(busy)return false;selected=mode;return true;}
static wireless_state wireless_snapshot(void){return network;}
static int wireless_setup_start(void){network.setup_stage=1;ap_requested=true;return 0;}
static bool wireless_setup_requested(void){return ap_requested;}
static void wireless_setup_stop(void){network.setup_ap=false;ap_requested=false;}
static void wireless_setup_credentials(char*a,size_t n,char*b,size_t k){snprintf(a,n,"StopWatch-setup");snprintf(b,k,"DEMO23456789");}
static void wireless_address(char*a,size_t n){snprintf(a,n,"10.0.0.123");}
static bool board_stop_pressed(void){return raw_stop;}
static bool identity_ready(void){return true;}
static bool mcp_ready(void){return true;}
static bool mcp_http_ready(void){return false;}
static int mcp_http_error(void){return 0x101;}
#define MALLOC_CAP_8BIT 1u
#define MALLOC_CAP_INTERNAL 2u
#define MALLOC_CAP_SPIRAM 4u
static unsigned heap_caps_get_free_size(unsigned caps){return caps==(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT)?2396:8121060;}
static unsigned heap_caps_get_minimum_free_size(unsigned caps){assert(caps==(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));return 1084;}
static unsigned heap_caps_get_largest_free_block(unsigned caps){assert(caps==(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));return 512;}
reset_trace_info reset_trace_previous(void){return (reset_trace_info){.valid=true,.stage=4,.error=0x101,.free_internal=2396,.largest_internal=512,.stack_free=768};}
static __attribute__((unused)) int esp_reset_reason(void){return 4;}
typedef struct {char version[32];} fixture_app_description;
static __attribute__((unused)) const fixture_app_description *esp_app_get_description(void){static const fixture_app_description d={"fixture"};return &d;}
bool actions_busy(void){return busy;}
static void mcp_stop_input(void){}
void actions_request_release(void){releases++;}
static void mcp_open_setup(void){}
static void mcp_close_setup(void){}
static void mcp_confirm_setup(void){}
static bool mcp_reset_identity(void){mcp_resets++;return true;}
static int mcp_pair_snapshot(char*a,size_t n,char*b,size_t k){snprintf(a,n,"123456");snprintf(b,k,"0123456789abcdef");return UI_PAIR_IDLE;}
static bool ble_input_connected(void){return true;}
static bool ble_input_ready(void){return true;}
static bool ble_input_forget(void){ble_resets++;return true;}
static bool touch_ready(void){return true;}
static int touch_error(void){return 0;}
'''
# bridge.h has external declarations; define mocked implementations with the same linkage.
stub=re.sub(r'static (bool|input_mode_t) (bridge_\w+|board_stop_pressed)',r'\1 \2',stub)
frames=r'''
typedef struct {bool pressed;unsigned x,y,event;} touch_point;
typedef struct {bool pressed;unsigned x,y;bool fail,stale;unsigned advance;} frame;
static frame frames[250];static unsigned frame_count,frame_index;static bool stale_display;
static bool touch_read(touch_point *p){
    frame f=frames[frame_index++];clock_ms+=f.advance;
    if(f.stale)atomic_fetch_add(&ui_revision,1);
    *p=(touch_point){.pressed=f.pressed,.x=f.x,.y=f.y};return !f.fail;
}
bool ui_displayed(ui_model*m){*m=model();if(stale_display)m->revision--;return true;}
static void add(bool pressed,unsigned x,unsigned y,bool fail,bool stale,unsigned advance){frames[frame_count++]=(frame){pressed,x,y,fail,stale,advance};}
static void clear_frames(void){frame_count=frame_index=0;add(false,0,0,false,false,0);}
'''
cases=r'''
static void setup_transition(bool previous_ap,wireless_state network){TRANSITION}
int main(void){
    atomic_store(&screen_ok,true);atomic_store(&page,UI_SETTINGS);
    activate(UI_CONNECT_OPEN,false,atomic_load(&ui_revision));assert(atomic_load(&page)==UI_CONNECT);
    ui_model connection=model();
    assert(connection.reset_reason==4&&connection.free_internal_heap==2396&&connection.uptime_seconds==clock_ms/1000);
    assert(connection.previous_ap.valid&&connection.previous_ap.stage==4&&connection.previous_ap.stack_free==768);
    activate(UI_WIFI_SETUP_OPEN,false,atomic_load(&ui_revision));assert(device_mode()==DEVICE_SETUP&&!network.setup_ap);
    ui_model pending=model();assert(pending.wifi_setup&&pending.wifi_setup_stage==1&&!pending.http_ready&&pending.http_error==0x101);
    assert(pending.free_internal_heap==2396&&pending.minimum_internal_heap==1084);
    assert(pending.largest_internal_block==512&&pending.free_psram==8121060);
    assert(pending.reset_reason==4&&!strcmp(pending.firmware_version,"fixture"));
    // Old AP closure must not cancel the new setup request between screen ticks.
    assert(wireless_setup_requested());
    setup_transition(true,network);assert(atomic_load(&page)==UI_PAIRING&&device_mode()==DEVICE_SETUP&&ap_requested);
    ap_requested=false;network.setup_error=0x101;
    setup_transition(true,network);assert(atomic_load(&page)==UI_PAIRING&&device_mode()==DEVICE_SETUP);
    network.setup_error=0;ap_requested=true;
    network.setup_ap=true;assert(model().wifi_setup&&model().setup_ap);
    activate(UI_BACK,false,atomic_load(&ui_revision));assert(atomic_load(&page)==UI_CONNECT&&!network.setup_ap&&device_mode()==DEVICE_MCP);
    assert(!model().wifi_setup);
    activate(UI_WIFI_SETUP_OPEN,false,atomic_load(&ui_revision));ap_requested=false;
    setup_transition(true,network);assert(atomic_load(&page)==UI_HOME&&device_mode()==DEVICE_MCP);
    activate(UI_CONNECT_OPEN,false,atomic_load(&ui_revision));
    activate(UI_SETUP_OPEN,false,atomic_load(&ui_revision));assert(!model().wifi_setup);
    activate(UI_BACK,false,atomic_load(&ui_revision));assert(atomic_load(&page)==UI_CONNECT);
    activate(UI_BACK,false,atomic_load(&ui_revision));assert(atomic_load(&page)==UI_SETTINGS);
    activate(UI_MORE_OPEN,false,atomic_load(&ui_revision));activate(UI_RESET_OPEN,false,atomic_load(&ui_revision));
    activate(UI_RESET_HOLD,false,atomic_load(&ui_revision));assert(mcp_resets==0);
    activate(UI_BACK,false,atomic_load(&ui_revision));assert(atomic_load(&page)==UI_MORE&&device_mode()==DEVICE_MCP);
    activate(UI_BLE_RESET_OPEN,false,atomic_load(&ui_revision));
    activate(UI_BLE_RESET_HOLD,false,atomic_load(&ui_revision));assert(ble_resets==0);
    ui_model a=model(),b=a;b.battery_percent=75;b.pressed=UI_BACK;b.hold_percent=50;b.eyes.eye[0].x=15;b.avatar.frame=42;
    assert(same_controls(&a,&b));b.revision++;assert(!same_controls(&a,&b));
    home();uint32_t stale=atomic_load(&ui_revision);device_stop();
    activate(UI_START,false,stale);assert(!device_control_granted());
    // A strictly targeted tap tolerates a small drift and release.
    clear_frames();add(true,200,270,false,false,0);add(true,70,270,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(device_control_granted());
    // Active blank/status/battery touches and sliding into Stop must not revoke input.
    unsigned before_releases=releases;
    clear_frames();add(true,200,100,false,false,0);add(true,200,270,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(device_control_granted()&&releases==before_releases);
    clear_frames();add(true,200,210,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(device_control_granted()&&releases==before_releases);
    busy=true;
    clear_frames();add(true,20,233,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(device_control_granted()&&releases==before_releases);busy=false;
    assert(face_touches>0);
    // The former Stop-button area only provides local avatar feedback, busy or idle.
    stale_display=true;
    clear_frames();add(true,200,270,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(device_control_granted());stale_display=false;
    clear_frames();add(true,200,270,false,false,0);add(true,200,270,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(device_control_granted()&&releases==before_releases);
    busy=true;
    clear_frames();add(true,200,270,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(device_control_granted()&&releases==before_releases);busy=false;
    // Either raw key stops immediately. Holding >700ms and release cannot navigate,
    // activate Home's Start, or reuse a stale focused Start button.
    for(unsigned key=1;key<=2;key++) {
        atomic_store(&focus,UI_START);key_count=180;key_start=clock_ms;
        for(unsigned i=0;i<key_count;i++)keys[i]=i<160?key:0;
        unsigned stopped=releases;buttons(NULL);
        assert(!device_control_granted()&&releases==stopped+1&&atomic_load(&page)==UI_HOME);
        activate(UI_START,false,atomic_load(&ui_revision));assert(device_control_granted());
    }
    online=false;key_count=2;key_start=clock_ms;keys[0]=keys[1]=0;buttons(NULL);
    assert(!device_control_granted());online=true;
    activate(UI_START,false,atomic_load(&ui_revision));atomic_store(&screen_ok,false);
    key_count=2;key_start=clock_ms;buttons(NULL);assert(!device_control_granted());atomic_store(&screen_ok,true);
    // I2C failures retain the existing fail-closed stop.
    activate(UI_START,false,atomic_load(&ui_revision));assert(device_control_granted());
    clear_frames();add(true,200,100,true,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(!device_control_granted());
    clear_frames();add(true,200,270,false,false,0);add(true,50,270,false,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(!device_control_granted());
    clear_frames();add(true,200,270,false,false,0);add(true,200,270,false,true,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(!device_control_granted());
    clear_frames();add(true,200,270,false,false,0);add(true,200,270,true,false,0);add(false,0,0,false,false,0);
    touch_input(NULL);assert(!device_control_granted());
    set_page(UI_BLE_RESET);
    clear_frames();add(true,200,270,false,false,0);add(true,70,270,false,false,2100);add(false,0,0,false,false,0);
    touch_input(NULL);assert(ble_resets==0); // resets have no slop
    clear_frames();add(true,200,270,false,false,0);add(true,200,270,false,false,2000);add(false,0,0,false,false,0);
    touch_input(NULL);assert(ble_resets==1&&!device_control_granted());
    assert(atomic_load(&touch_feedback)==0);
    printf("Actual control/touch: all control touches ignored, both raw keys stop/consume release, input/display/read-error stop, navigation/reset/drift: passed\n");
}
'''
cases=cases.replace('TRANSITION',transition)
# Isolate the actual optional read adapter with a fake, read-only I2C call.
board_stub=r'''
#include "bridge.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#define ESP_OK 0
static void *board_power=(void*)1;
static bool fail;static unsigned reads;
static int i2c_master_transmit_receive(void*dev,const uint8_t*reg,size_t n,uint8_t*out,size_t len,int timeout){
    assert(dev==board_power&&*reg==0x22&&n==1&&len==2&&timeout<=20);reads++;
    out[0]=0x88;out[1]=0x0f;return fail?-1:0;
}
'''
board_cases=r'''
int main(void){uint16_t mv=0;assert(board_battery_voltage(&mv)&&mv==3976);fail=true;assert(!board_battery_voltage(&mv));
board_power=NULL;assert(!board_battery_voltage(&mv));assert(!board_battery_voltage(NULL));assert(reads==2);
puts("Actual M5PM1 read adapter: register, length, little endian, timeout and failure: passed");}
'''
with tempfile.TemporaryDirectory(prefix='anan-ui-control-') as work:
    for name,code in [('control',stub+prefix+frames+buttons+touch+cases),('battery-read',board_stub+board+board_cases)]:
        c=Path(work)/(name+'.c');c.write_text(code);exe=Path(work)/name
        subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',*include_flags(),str(c),str(root/'main/ui/ui_layout.c'),str(root/'main/ui/eyes.c'),'-lm','-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)

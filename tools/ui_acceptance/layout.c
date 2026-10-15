// SPDX-License-Identifier: Apache-2.0
// Defined before the large-target implementation; synthetic host data only.
#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned width(const char *s, unsigned scale) {
    unsigned n=0;
    while(*s) {
        if((unsigned char)*s<128){n+=6*scale;s++;}
        else {n+=scale==1?18:scale>=3?28:24;s++;while(((unsigned char)*s&0xc0)==0x80)s++;}
    }
    return n;
}
static void check(ui_model *m) {
    ui_widget w[UI_WIDGET_LIMIT];size_t n=ui_layout(m,w);unsigned buttons=0;
    assert(n<=UI_WIDGET_LIMIT);
    for(size_t i=0;i<n;i++) {
        assert(width(w[i].text,w[i].scale)<=(unsigned)w[i].rect.w);
        if(!w[i].button)continue;
        buttons++;assert(w[i].rect.h>=72);
        for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++){
            int x=2*(w[i].rect.x+dx*(w[i].rect.w-1))-465;
            int y=2*(w[i].rect.y+dy*(w[i].rect.h-1))-465;
            assert(x*x+y*y<466*466);
        }
        if(w[i].enabled)assert(ui_hit(m,w[i].rect.x+w[i].rect.w/2,w[i].rect.y+w[i].rect.h/2)==w[i].action);
        for(size_t j=0;j<i;j++)if(w[j].button)
            assert(w[i].rect.y>=w[j].rect.y+w[j].rect.h||w[j].rect.y>=w[i].rect.y+w[i].rect.h||
                   w[i].rect.x>=w[j].rect.x+w[j].rect.w||w[j].rect.x>=w[i].rect.x+w[i].rect.w);
    }
    assert(buttons<=4);
}
int main(void){
    ui_model m={.identity_ready=true,.mcp_ready=true,.usb_online=true,.input_online=true,
        .ble_ready=true,.touch_ready=true,.battery_percent=75};
    strcpy(m.code,"123456");strcpy(m.wifi_address,"192.168.100.123");
    strcpy(m.ap_name,"StopWatch-setup");strcpy(m.ap_password,"DEMO23456789");
    for(int lang=0;lang<2;lang++)for(int page=0;page<=UI_BLE_RESET;page++)
        for(int pair=0;pair<=UI_PAIR_EXPIRED;pair++)for(int ap=0;ap<2;ap++){
            m.language=lang;m.page=page;m.pairing=pair;m.setup_ap=ap;check(&m);
        }
    for(int lang=0;lang<2;lang++)for(int stage=0;stage<8;stage++)for(int error=0;error<2;error++){
        m.language=lang;m.page=UI_PAIRING;m.pairing=UI_PAIR_IDLE;m.setup_ap=false;
        m.wifi_setup=true;m.wifi_setup_stage=stage;m.wifi_setup_error=error?0x101:0;
        m.http_ready=false;m.http_error=0x101;
        m.free_internal_heap=2396;m.minimum_internal_heap=1084;
        m.largest_internal_block=512;m.free_psram=8121060;check(&m);
        strcpy(m.firmware_version,"0.9.8-reset-info");m.reset_reason=4;m.uptime_seconds=123456;check(&m);
        ui_widget scene[UI_WIDGET_LIMIT];size_t n=ui_layout(&m,scene);bool diagnostic=false,internal=false,external=false;
        for(size_t i=0;i<n;i++){
            assert(!strstr(scene[i].text,"连接MCP应用")&&!strstr(scene[i].text,"Connect MCP app"));
            if(strstr(scene[i].text,"AP:"))diagnostic=true;
            if(strstr(scene[i].text,"INT: 2396 B"))internal=true;
            if(strstr(scene[i].text,"PSRAM: 8121060 B"))external=true;
        }
        assert(diagnostic&&internal&&external);
        m.free_internal_heap=999999;m.minimum_internal_heap=999999;
        m.largest_internal_block=999999;m.free_psram=8388608;check(&m);
    }
    m.wifi_setup=false;
    for(int lang=0;lang<2;lang++){
        m.language=lang;m.page=UI_CONNECT;m.reset_reason=4;m.uptime_seconds=123456;
        m.previous_ap=(reset_trace_info){.valid=true,.stage=4,.error=0x101,.free_internal=2396,.largest_internal=512,.stack_free=768};
        check(&m);ui_widget scene[UI_WIDGET_LIMIT];size_t n=ui_layout(&m,scene);
        bool reset=false,previous=false,heap=false;
        for(size_t i=0;i<n;i++){
            if(strstr(scene[i].text,"RST:4")&&strstr(scene[i].text,"UP:123456S"))reset=true;
            if(strstr(scene[i].text,"PREV:MODE")&&strstr(scene[i].text,"H:2396")&&strstr(scene[i].text,"S:768"))previous=true;
            if(strstr(scene[i].text,"INT:"))heap=true;
            if(scene[i].button&&scene[i].action==UI_WIFI_SETUP_OPEN)assert(scene[i].rect.y==140&&scene[i].rect.h==84);
            if(scene[i].button&&scene[i].action==UI_SETUP_OPEN)assert(scene[i].rect.y==240&&scene[i].rect.h==84);
        }
        assert(reset&&previous&&heap);
    }
    m.page=UI_SETTINGS;m.setup_ap=false;
    assert(ui_parent(UI_INPUT)==UI_SETTINGS);assert(ui_parent(UI_MORE)==UI_SETTINGS);
    assert(ui_parent(UI_LANGUAGE)==UI_MORE);assert(ui_parent(UI_RESET)==UI_MORE);
    assert(ui_parent(UI_BLE_RESET)==UI_MORE);assert(ui_parent(UI_SETTINGS)==UI_HOME);
    assert(!ui_target_contains(&m,UI_INPUT_OPEN,72,130,0));
    assert(ui_target_contains(&m,UI_INPUT_OPEN,72,130,10));
    assert(!ui_target_contains(&m,UI_INPUT_OPEN,60,130,10));
    assert(!ui_target_contains(&m,UI_INPUT_OPEN,200,220,100)); // never retarget neighbor
    m.page=UI_HOME;m.control_granted=true;
    ui_widget w[UI_WIDGET_LIMIT];size_t n=ui_layout(&m,w);
    assert(n==0); // Eyes-only control view has no text, buttons or touch targets.
    for(unsigned y=0;y<466;y++)for(unsigned x=0;x<466;x++)assert(ui_hit(&m,x,y)==UI_NONE);
    for(int page=UI_SETTINGS;page<=UI_BLE_RESET;page++){
        m.page=page;n=ui_layout(&m,w);
        for(size_t i=0;i<n;i++)assert(!strstr(w[i].text,"Battery")&&!strstr(w[i].text,"电量"));
    }
    puts("Large targets, both-language labels, circle bounds, parent navigation, slop and home-only battery: passed");
}

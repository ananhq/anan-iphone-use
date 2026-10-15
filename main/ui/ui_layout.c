// SPDX-License-Identifier: Apache-2.0
#include "ui.h"
#include <stdio.h>
#include <string.h>
#define INK 0xe71c
#define MUTED 0x9d35
#define CARD 0x1966
#define ACCENT 0xfe89
#define DANGER 0xf28b
#define GREEN 0x46b1
static const char *tr(const ui_model *m, const char *zh, const char *en) {
    return m->language == UI_ZH ? zh : en;
}
static void text(ui_widget *w, size_t *n, int y, const char *label, unsigned scale,
                 uint16_t color) {
    ui_widget *v = &w[(*n)++];
    *v = (ui_widget){.rect = {35, y, 396, scale >= 3 ? (int)scale * 7 > 28 ? (int)scale * 7 : 28
                                                   : scale == 1 ? 18 : 24},
                     .scale = scale, .foreground = color};
    snprintf(v->text, sizeof(v->text), "%s", label);
}
static void button(ui_widget *w, size_t *n, int x, int y, int width, int height,
                   const char *label, ui_action action, bool enabled, bool selected, uint16_t color) {
    ui_widget *v = &w[(*n)++];
    *v = (ui_widget){.rect = {x,y,width,height}, .scale = 3,
                     .foreground = color == ACCENT ? 0x10a3 : INK,
                     .background = enabled ? color : 0x18e4, .action = action,
                     .button = true, .enabled = enabled, .selected = selected};
    snprintf(v->text, sizeof(v->text), "%s", label);
}
static void row(ui_widget *w,size_t *n,int y,int h,const char *label,ui_action action,bool enabled,
                bool selected,uint16_t color) {
    button(w,n,73,y,320,h,label,action,enabled,selected,color);
}
static void back(const ui_model *m,ui_widget *w,size_t *n,bool enabled) {
    button(w,n,137,356,192,72,tr(m,"返回","Back"),UI_BACK,enabled,false,CARD);
}
size_t ui_layout(const ui_model *m, ui_widget w[UI_WIDGET_LIMIT]) {
    memset(w,0,sizeof(*w)*UI_WIDGET_LIMIT);
    size_t n=0;char line[96];
    bool reachable=m->usb_online||m->wifi_connected||m->setup_ap;
    if(m->page==UI_HOME&&m->control_granted)return 0; // Pure eyes; no touch targets.
    if(m->page==UI_HOME){
        bool active=m->control_granted||m->input_busy;
        text(w,&n,49,"StopWatch",3,INK);
        text(w,&n,94,m->input_mode==INPUT_BLE ?
             tr(m,m->input_online?"蓝牙输入已连接":m->ble_connected?"蓝牙已连接 等待输入":"等待蓝牙连接",
                  m->input_online?"BLE input connected":m->ble_connected?"BLE connected / wait HID":"Waiting for Bluetooth"):
             tr(m,m->usb_online?"USB已连接":"请连接USB线",
                  m->usb_online?"USB connected":"Connect USB cable"),2,
             m->input_online?GREEN:m->input_mode==INPUT_BLE&&m->ble_connected?ACCENT:MUTED);
        if(m->wifi_connected)snprintf(line,sizeof(line),"Wi-Fi: %s",m->wifi_address);
        else snprintf(line,sizeof(line),"%s",tr(m,m->wifi_configured?"Wi-Fi连接中":"设置中可配置Wi-Fi",
                                                   m->wifi_configured?"Wi-Fi connecting":"Set Wi-Fi in Settings"));
        text(w,&n,128,line,2,MUTED);
        bool paired=m->identity_ready&&m->mcp_ready;
        text(w,&n,164,tr(m,!paired?"先完成MCP配对":m->input_online?"准备就绪":"等待输入连接",
                           !paired?"Pair your MCP app first":m->input_online?"Ready":"Waiting for input"),2,MUTED);
        if(m->battery_percent<0)snprintf(line,sizeof(line),"%s",tr(m,"电量未知","Battery unknown"));
        else snprintf(line,sizeof(line),tr(m,"电量约 %d%%","Battery est. %d%%"),m->battery_percent);
        text(w,&n,198,line,2,m->battery_percent>=0&&m->battery_percent<=20?DANGER:MUTED);
        ui_action action=!m->identity_ready||!m->mcp_ready?UI_SETUP_OPEN:UI_START;
        const char *label=tr(m,action==UI_SETUP_OPEN?"连接MCP应用":"开始控制",
                              action==UI_SETUP_OPEN?"Connect MCP app":"Start control");
        row(w,&n,234,96,label,action,!active&&(action==UI_SETUP_OPEN?reachable:m->input_online),false,ACCENT);
        if(!active)button(w,&n,121,350,224,72,tr(m,"设置","Settings"),UI_SETTINGS_OPEN,true,false,CARD);

    }else if(m->page==UI_SETTINGS){
        text(w,&n,49,tr(m,"设置","Settings"),3,INK);
        row(w,&n,100,72,tr(m,"输入方式","Phone input"),UI_INPUT_OPEN,true,false,CARD);
        row(w,&n,184,72,tr(m,"连接与配网","Connect / Wi-Fi"),UI_CONNECT_OPEN,true,false,CARD);
        row(w,&n,268,72,tr(m,"更多设置","More settings"),UI_MORE_OPEN,true,false,CARD);
        back(m,w,&n,true);
    }else if(m->page==UI_INPUT){
        text(w,&n,49,tr(m,"输入方式","Phone input"),3,INK);
        text(w,&n,98,tr(m,m->input_mode==INPUT_BLE?"当前：蓝牙 BLE":"当前：USB 有线",
                         m->input_mode==INPUT_BLE?"Current: Bluetooth LE":"Current: USB cable"),2,MUTED);
        row(w,&n,140,84,tr(m,"USB 有线","USB cable"),UI_USB_SELECT,!m->setup_ap,m->input_mode==INPUT_USB,CARD);
        row(w,&n,240,84,tr(m,"蓝牙 BLE","Bluetooth LE"),UI_BLE_SELECT,m->ble_ready&&!m->setup_ap,m->input_mode==INPUT_BLE,CARD);
        back(m,w,&n,true);
    }else if(m->page==UI_CONNECT){
        text(w,&n,49,tr(m,"连接与配网","Connect / Wi-Fi"),3,INK);
        text(w,&n,79,m->firmware_version,1,MUTED);
        if(m->wifi_connected)snprintf(line,sizeof(line),"Wi-Fi: %s",m->wifi_address);
        else snprintf(line,sizeof(line),"%s",tr(m,m->wifi_configured?"Wi-Fi连接中":"尚未配置Wi-Fi",
                                                   m->wifi_configured?"Wi-Fi connecting":"Wi-Fi not configured"));
        text(w,&n,98,line,2,m->wifi_connected?GREEN:MUTED);
        snprintf(line,sizeof(line),"HTTP:%s RST:%d UP:%luS",m->http_ready?"ON":"OFF",m->reset_reason,(unsigned long)m->uptime_seconds);
        text(w,&n,120,line,1,m->http_ready?GREEN:DANGER);
        row(w,&n,140,84,tr(m,"配置 Wi-Fi","Set up Wi-Fi"),UI_WIFI_SETUP_OPEN,true,false,CARD);
        if(m->previous_ap.valid){
            static const char *const stages[]={"IDLE","QUEUED","RELEASE","BLE","MODE","CONFIG","START","READY"};
            snprintf(line,sizeof(line),"PREV:%s E:%lX H:%lu S:%lu",
                     m->previous_ap.stage<8?stages[m->previous_ap.stage]:"?",
                     (unsigned long)(uint32_t)m->previous_ap.error,(unsigned long)m->previous_ap.free_internal,
                     (unsigned long)m->previous_ap.stack_free);
        }else snprintf(line,sizeof(line),"PREV: NONE");
        text(w,&n,225,line,1,MUTED);w[n-1].rect.h=14;
        row(w,&n,240,84,tr(m,"连接 MCP 应用","Connect MCP app"),UI_SETUP_OPEN,reachable,false,CARD);
        snprintf(line,sizeof(line),"INT:%lu MIN:%lu MAX:%lu B",(unsigned long)m->free_internal_heap,
                 (unsigned long)m->minimum_internal_heap,(unsigned long)m->largest_internal_block);
        text(w,&n,331,line,1,MUTED);
        back(m,w,&n,true);
    }else if(m->page==UI_MORE){
        text(w,&n,49,tr(m,"更多设置","More settings"),3,INK);
        row(w,&n,100,72,"语言 / Language",UI_LANGUAGE_OPEN,true,false,CARD);
        row(w,&n,184,72,tr(m,"蓝牙重新配对","Pair Bluetooth"),UI_BLE_RESET_OPEN,m->ble_ready&&!m->setup_ap,false,CARD);
        row(w,&n,268,72,tr(m,"MCP 重置","Reset MCP"),UI_RESET_OPEN,true,false,CARD);
        back(m,w,&n,true);
    }else if(m->page==UI_LANGUAGE){
        text(w,&n,49,"语言 / Language",3,INK);
        row(w,&n,140,84,"简体中文",UI_ZH_SELECT,true,m->language==UI_ZH,CARD);
        row(w,&n,240,84,"English",UI_EN_SELECT,true,m->language==UI_EN,CARD);
        back(m,w,&n,true);
    }else if(m->page==UI_BLE_RESET||m->page==UI_RESET){
        bool ble=m->page==UI_BLE_RESET;
        text(w,&n,49,tr(m,ble?"蓝牙重新配对":"重置 MCP 配对",ble?"Pair Bluetooth":"Reset MCP"),3,INK);
        text(w,&n,127,tr(m,ble?"仅清除蓝牙绑定":"需要重新导入MCP配置",
                             ble?"Clear Bluetooth bond only":"Import new MCP config"),2,MUTED);
        text(w,&n,169,tr(m,ble?"Wi-Fi和MCP配置保留":"保留Wi-Fi和蓝牙绑定",
                             ble?"Keep Wi-Fi / MCP settings":"Keep Wi-Fi / Bluetooth"),2,MUTED);
        row(w,&n,248,88,tr(m,"按住2秒重置","Hold 2s to reset"),ble?UI_BLE_RESET_HOLD:UI_RESET_HOLD,
            !ble||(m->ble_ready&&!m->setup_ap),false,DANGER);
        back(m,w,&n,true);
    }else if(m->page==UI_PAIRING){
        if(m->wifi_setup&&!m->setup_ap){
            static const char *const stages[]={"IDLE","QUEUED","RELEASE","BLE","MODE","CONFIG","START","READY"};
            unsigned stage=(unsigned)m->wifi_setup_stage;
            text(w,&n,49,tr(m,"配置 Wi-Fi","Set up Wi-Fi"),3,INK);
            text(w,&n,84,m->firmware_version,1,MUTED);
            text(w,&n,117,tr(m,m->wifi_setup_error?"Wi-Fi未就绪":"请稍候",
                              m->wifi_setup_error?"Wi-Fi not ready":"Please wait"),2,m->wifi_setup_error?DANGER:MUTED);
            snprintf(line,sizeof(line),"AP: %s",stage<sizeof(stages)/sizeof(*stages)?stages[stage]:"?");
            text(w,&n,155,line,2,ACCENT);
            snprintf(line,sizeof(line),"Error: 0x%X",(unsigned)m->wifi_setup_error);text(w,&n,191,line,2,MUTED);
            snprintf(line,sizeof(line),m->http_ready?"HTTP: ON":"HTTP: OFF (0x%X)",(unsigned)m->http_error);
            text(w,&n,227,line,2,m->http_ready?GREEN:DANGER);
            snprintf(line,sizeof(line),"INT: %lu B",(unsigned long)m->free_internal_heap);text(w,&n,259,line,2,MUTED);
            snprintf(line,sizeof(line),"MIN: %lu  MAX: %lu B",(unsigned long)m->minimum_internal_heap,
                     (unsigned long)m->largest_internal_block);text(w,&n,287,line,1,MUTED);
            snprintf(line,sizeof(line),"PSRAM: %lu B",(unsigned long)m->free_psram);text(w,&n,310,line,1,MUTED);
            snprintf(line,sizeof(line),"RESET: %d  UP: %luS",m->reset_reason,(unsigned long)m->uptime_seconds);
            text(w,&n,333,line,1,MUTED);
        }else if(m->pairing==UI_PAIR_PENDING){
            text(w,&n,49,tr(m,"确认MCP配对","Confirm pairing"),3,INK);
            text(w,&n,113,tr(m,"核对客户端验证码","Match code on client"),2,MUTED);
            text(w,&n,171,m->code,7,ACCENT);
            row(w,&n,260,78,tr(m,"确认配对","Confirm pairing"),UI_CONFIRM_PAIR,true,false,ACCENT);
        }else if(m->pairing==UI_PAIR_READY){
            text(w,&n,49,tr(m,"配置已就绪","MCP ready"),3,INK);
            text(w,&n,126,tr(m,"在手机复制MCP配置","Copy MCP config on phone"),2,MUTED);
            text(w,&n,166,tr(m,"导入Agent后即可使用","Import into your agent"),2,MUTED);
            text(w,&n,206,tr(m,m->setup_ap?"复制后返回关闭热点":"手机完成后再返回",
                                m->setup_ap?"Copy before closing hotspot":"Return after importing"),2,MUTED);
            row(w,&n,260,88,tr(m,"完成 返回首页","Done / Home"),UI_SETUP_DONE,true,false,ACCENT);
        }else if(m->pairing==UI_PAIR_WORKING){
            text(w,&n,49,tr(m,"正在准备配对","Pairing setup"),3,INK);
            text(w,&n,190,tr(m,"请稍候","Please wait"),2,MUTED);
        }else if(m->pairing==UI_PAIR_FAILED||m->pairing==UI_PAIR_EXPIRED){
            text(w,&n,49,tr(m,"请重新开始配对","Restart pairing"),3,INK);
            text(w,&n,142,tr(m,m->pairing==UI_PAIR_FAILED?"配对未完成":"配对已超时",
                                m->pairing==UI_PAIR_FAILED?"Pairing did not complete":"Pairing window expired"),2,MUTED);
            row(w,&n,248,88,tr(m,"重新开始","Restart setup"),UI_SETUP_OPEN,true,false,ACCENT);
        }else if(m->setup_ap){
            text(w,&n,49,tr(m,"扫码连接热点","Join Wi-Fi"),3,INK);
            w[n++]=(ui_widget){.rect={130,95,205,205},.qr=true};
            text(w,&n,308,m->ap_password,2,ACCENT);
            text(w,&n,333,"http://192.168.4.1",2,INK);
        }else{
            text(w,&n,49,tr(m,"连接MCP应用","Connect MCP app"),3,INK);
            text(w,&n,125,tr(m,"手机Safari打开","Open Safari on phone"),2,MUTED);
            snprintf(line,sizeof(line),"http://%s",m->usb_online?"192.168.7.1":m->wifi_address);
            text(w,&n,167,line,2,ACCENT);
            text(w,&n,222,tr(m,"网页点击开始配对","Start pairing on web page"),2,MUTED);
            text(w,&n,262,tr(m,"再在设备确认验证码","Then confirm matching code"),2,MUTED);
        }
        back(m,w,&n,m->pairing!=UI_PAIR_WORKING);
    }else{
        text(w,&n,86,tr(m,"正在重置配对","Resetting pairing"),3,INK);
        text(w,&n,195,tr(m,"请稍候","Please wait"),2,MUTED);
    }
    return n;
}
ui_action ui_hit(const ui_model *m,unsigned x,unsigned y){
    ui_widget w[UI_WIDGET_LIMIT];size_t n=ui_layout(m,w);
    for(size_t i=0;i<n;i++)if(w[i].button&&w[i].enabled&&x>=(unsigned)w[i].rect.x&&
        x<(unsigned)(w[i].rect.x+w[i].rect.w)&&y>=(unsigned)w[i].rect.y&&y<(unsigned)(w[i].rect.y+w[i].rect.h))
        return w[i].action;
    return UI_NONE;
}
bool ui_target_contains(const ui_model *m,ui_action target,unsigned x,unsigned y,unsigned margin){
    if(target==UI_NONE||x>=466||y>=466)return false;
    ui_widget w[UI_WIDGET_LIMIT];size_t n=ui_layout(m,w);
    for(size_t i=0;i<n;i++)if(w[i].button&&w[i].enabled&&w[i].action!=target&&
        (int)x>=w[i].rect.x&&(int)x<w[i].rect.x+w[i].rect.w&&
        (int)y>=w[i].rect.y&&(int)y<w[i].rect.y+w[i].rect.h)return false;
    for(size_t i=0;i<n;i++)if(w[i].button&&w[i].enabled&&w[i].action==target)
        return (int)x>=w[i].rect.x-(int)margin&&(int)x<w[i].rect.x+w[i].rect.w+(int)margin&&
               (int)y>=w[i].rect.y-(int)margin&&(int)y<w[i].rect.y+w[i].rect.h+(int)margin;
    return false;
}
ui_page ui_parent(ui_page page){
    switch(page){
    case UI_INPUT:case UI_CONNECT:case UI_MORE:return UI_SETTINGS;
    case UI_LANGUAGE:case UI_RESET:case UI_BLE_RESET:return UI_MORE;
    default:return UI_HOME;
    }
}
ui_action ui_next_focus(const ui_model *m){
    ui_widget w[UI_WIDGET_LIMIT];size_t n=ui_layout(m,w);ui_action first=UI_NONE;
    bool after=m->focus==UI_NONE;
    for(size_t i=0;i<n;i++)if(w[i].button&&w[i].enabled){
        if(first==UI_NONE)first=w[i].action;
        if(after)return w[i].action;
        if(w[i].action==m->focus)after=true;
    }
    return first;
}

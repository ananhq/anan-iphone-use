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
    *v = (ui_widget){.rect = {35, y, 396,
                              scale >= 3   ? (int)scale * 7 > 28 ? (int)scale * 7 : 28
                              : scale == 1 ? 18
                                           : 24},
                     .scale = scale,
                     .foreground = color};
    snprintf(v->text, sizeof(v->text), "%s", label);
}
static void button(ui_widget *w, size_t *n, int x, int y, int width, int height, const char *label,
                   ui_action action, bool enabled, bool selected, uint16_t color) {
    ui_widget *v = &w[(*n)++];
    *v = (ui_widget){.rect = {x, y, width, height},
                     .scale = 2,
                     .foreground = color == ACCENT ? 0x10a3 : INK,
                     .background = enabled ? color : 0x18e4,
                     .action = action,
                     .button = true,
                     .enabled = enabled,
                     .selected = selected};
    snprintf(v->text, sizeof(v->text), "%s", label);
}
size_t ui_layout(const ui_model *m, ui_widget w[UI_WIDGET_LIMIT]) {
    memset(w, 0, sizeof(*w) * UI_WIDGET_LIMIT);
    size_t n = 0;
    char line[96];
    if (m->page == UI_HOME) {
        text(w, &n, 55, "StopWatch", 3, INK);
        text(w, &n, 96,
             m->input_mode==INPUT_BLE ? tr(m,m->input_online ? "蓝牙输入已连接" : "请在手机配对蓝牙",
                                          m->input_online ? "BLE input connected" : "Pair Bluetooth on phone") :
             tr(m, m->usb_online ? "USB已连接" : "请连接USB线",
                m->usb_online ? "USB connected" : "Connect USB cable"),
             2, m->input_online ? GREEN : MUTED);
        snprintf(line,sizeof(line),m->wifi_connected ? "Wi-Fi: %s" : "%s",
                 m->wifi_connected ? m->wifi_address : tr(m,m->wifi_configured ? "Wi-Fi连接中" : "USB网页配置Wi-Fi",
                                                                              m->wifi_configured ? "Wi-Fi connecting" : "Set Wi-Fi on USB page"));
        text(w,&n,128,line,2,MUTED);
        text(w, &n, 168, tr(m, "MCP控制", "MCP control"), 3, INK);
        if (m->control_granted) {
            snprintf(line, sizeof(line), "%s", tr(m, "控制已开启", "Control enabled"));
        } else
            snprintf(line, sizeof(line), "%s",
                     tr(m, m->identity_ready ? "点击开始控制" : "先完成MCP配对",
                        m->identity_ready ? "Tap to start control" : "Pair your MCP app first"));
        text(w, &n, 240, line, 2, MUTED);
        ui_action main_action;
        const char *label;
        bool enabled = m->input_online;
        if (m->control_granted || m->input_busy) {
            main_action = UI_STOP;
            label = tr(m, "停止控制", "Stop control");
        } else if (!m->identity_ready || !m->mcp_ready) {
            main_action = UI_SETUP_OPEN;
            label = tr(m, "连接MCP应用", "Connect MCP app");
            enabled=m->usb_online || m->wifi_connected || m->setup_ap;
        } else {
            main_action = UI_START;
            label = tr(m, "开始控制", "Start control");
        }
        button(w, &n, 73, 285, 320, 64, label, main_action, enabled, false,
               main_action == UI_STOP ? DANGER : ACCENT);
        button(w, &n, 107, 365, 252, 48, tr(m, "设置 / Settings", "Settings / 设置"),
               UI_SETTINGS_OPEN, true, false, CARD);
        text(w, &n, 416,
             tr(m, m->touch_ready ? "触屏或按键停止" : "黄键选 蓝键确认",
                m->touch_ready ? "Touch or keys stop" : "A next / B OK"),
             2, MUTED);
    } else if (m->page == UI_SETTINGS) {
        text(w, &n, 58, tr(m, "设置 / Settings", "Settings / 设置"), 2, INK);
        text(w, &n, 98, "Language / 语言", 2, MUTED);
        button(w, &n, 73, 134, 154, 56, "简体中文", UI_ZH_SELECT, true, m->language == UI_ZH, CARD);
        button(w, &n, 239, 134, 154, 56, "English", UI_EN_SELECT, true, m->language == UI_EN, CARD);
        button(w,&n,73,207,320,50,tr(m,"输入 / 网络","Input / network"),UI_INPUT_OPEN,true,false,CARD);
        button(w, &n, 73, 269, 320, 50, tr(m, "连接MCP应用", "Connect MCP app"), UI_SETUP_OPEN,
               m->usb_online || m->wifi_connected || m->setup_ap, false, CARD);
        button(w, &n, 97, 331, 272, 44, tr(m, "重置配对", "Reset pairing"), UI_RESET_OPEN, true,
               false, CARD);
        button(w, &n, 122, 389, 222, 42, tr(m, "返回", "Back"), UI_BACK, true, false, CARD);
    } else if (m->page==UI_INPUT) {
        text(w,&n,58,tr(m,"输入 / 网络","Input / network"),2,INK);
        text(w,&n,102,tr(m,"选择手机输入方式","Select phone input"),2,MUTED);
        button(w,&n,73,144,154,56,"USB",UI_USB_SELECT,!m->setup_ap,m->input_mode==INPUT_USB,CARD);
        button(w,&n,239,144,154,56,"BLE",UI_BLE_SELECT,m->ble_ready&&!m->setup_ap,m->input_mode==INPUT_BLE,CARD);
        text(w,&n,215,"BLE: Anan StopWatch",2,MUTED);
        text(w,&n,248,tr(m,"手机开启AssistiveTouch","Enable AssistiveTouch"),2,MUTED);
        button(w,&n,97,285,272,52,tr(m,"按住2秒重置蓝牙","Hold 2s: reset BLE"),
               UI_BLE_RESET_HOLD,m->ble_ready,false,CARD);
        button(w,&n,97,344,272,40,tr(m,"无线配网 / 管理Wi-Fi","Wi-Fi setup / manage"),UI_WIFI_SETUP_OPEN,true,false,CARD);
        button(w,&n,122,390,222,42,tr(m,"返回","Back"),UI_BACK,true,false,CARD);
    } else if (m->page == UI_PAIRING) {
        if (m->pairing == UI_PAIR_PENDING) {
            text(w, &n, 65, tr(m, "确认配对", "Pair device"), 3, INK);
            text(w, &n, 123, tr(m, "核对手机验证码", "Match the code on phone"), 2, MUTED);
            text(w, &n, 176, m->code, 7, ACCENT);
            button(w, &n, 73, 282, 320, 64, tr(m, "确认配对", "Confirm pairing"), UI_CONFIRM_PAIR,
                   true, false, ACCENT);
        } else if (m->pairing == UI_PAIR_READY) {
            text(w, &n, 65, tr(m, "继续在手机配置", "Continue on phone"), 2, INK);
            text(w, &n, 128, tr(m, "手机复制MCP配置", "Copy MCP config on phone"), 2, MUTED);
            text(w, &n, 166, tr(m, "导出配置到Anan", "Import config into Anan"), 2, MUTED);
            text(w, &n, 204, tr(m, m->setup_ap?"复制后返回关闭热点":"手机完成后再返回",m->setup_ap?"Copy, return to close Wi-Fi":"Return after importing"), 2, MUTED);
            button(w, &n, 73, 295, 320, 62, tr(m, "已导入 返回", "Imported, go back"),
                   UI_SETUP_DONE, true, false, ACCENT);
        } else if (m->pairing == UI_PAIR_WORKING) {
            text(w, &n, 76, tr(m, "正在准备配对", "Preparing pairing"), 2, INK);
            text(w, &n, 190, tr(m, "正在准备设备配对", "Preparing device pairing"), 2, MUTED);
            text(w, &n, 233, tr(m, "请稍候", "Please wait"), 2, MUTED);
        } else {
            text(w, &n, 65, m->setup_ap?m->ap_name:tr(m, "连接MCP应用", "Connect MCP app"), 2, INK);
            if(m->setup_ap){
                text(w,&n,98,tr(m,"手机扫码连接热点","Scan to join Wi-Fi"),2,MUTED);
                w[n++]=(ui_widget){.rect={130,125,205,205},.qr=true};
                snprintf(line,sizeof(line),"%s: %s",tr(m,"密码","Password"),m->ap_password);
                text(w,&n,338,line,1,ACCENT);
                text(w,&n,358,"http://192.168.4.1",2,INK);
            }else{
                text(w,&n,125,tr(m,"手机Safari打开","Open Safari on phone"),2,MUTED);
                snprintf(line,sizeof(line),"http://%s",m->usb_online?"192.168.7.1":m->wifi_address);
                text(w,&n,167,line,2,ACCENT);
                text(w,&n,222,tr(m,"手机点击开始配对","Start pairing on phone"),2,MUTED);
                text(w,&n,262,tr(m,"再在设备确认验证码","Then confirm matching code"),2,MUTED);
            }
            if (m->pairing == UI_PAIR_FAILED || m->pairing == UI_PAIR_EXPIRED)
                button(w, &n, 97, 310, 272, 48, tr(m, "重新开始", "Restart setup"), UI_SETUP_OPEN,
                       true, false, ACCENT);
        }
        button(w, &n, 122, 380, 222, 46, tr(m, "返回", "Back"), UI_BACK,
               m->pairing != UI_PAIR_WORKING, false, CARD);
    } else if (m->page == UI_RESET) {
        text(w, &n, 67, tr(m, "重置配对", "Reset pairing"), 3, INK);
        text(w, &n, 150, tr(m, "会清除配对密钥", "Pairing key will change"), 2, MUTED);
        text(w, &n, 190, tr(m, "需要重新导入配置", "Import the new config"), 2, MUTED);
        button(w, &n, 73, 278, 320, 66, tr(m, "按住2秒重置", "Hold 2s to reset"), UI_RESET_HOLD,
               true, false, DANGER);
        button(w, &n, 122, 380, 222, 46, tr(m, "取消", "Cancel"), UI_BACK, true, false, CARD);
    } else {
        text(w, &n, 86, tr(m, "正在重置配对", "Resetting pairing"), 2, INK);
        text(w, &n, 195, tr(m, "请稍候", "Please wait"), 2, MUTED);
    }
    return n;
}
ui_action ui_hit(const ui_model *m, unsigned x, unsigned y) {
    ui_widget w[UI_WIDGET_LIMIT];
    size_t n = ui_layout(m, w);
    for (size_t i = 0; i < n; ++i)
        if (w[i].button && w[i].enabled && x >= (unsigned)w[i].rect.x &&
            x < (unsigned)(w[i].rect.x + w[i].rect.w) && y >= (unsigned)w[i].rect.y &&
            y < (unsigned)(w[i].rect.y + w[i].rect.h))
            return w[i].action;
    return UI_NONE;
}
ui_action ui_next_focus(const ui_model *m) {
    ui_widget w[UI_WIDGET_LIMIT];
    size_t n = ui_layout(m, w);
    ui_action first = UI_NONE;
    bool after = m->focus == UI_NONE;
    for (size_t i = 0; i < n; ++i)
        if (w[i].button && w[i].enabled) {
            if (first == UI_NONE)
                first = w[i].action;
            if (after)
                return w[i].action;
            if (w[i].action == m->focus)
                after = true;
        }
    return first;
}

// SPDX-License-Identifier: Apache-2.0
// Static document-layout export only: no board, transport, agent or input runtime.
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void string(const char *s) {
    putchar('"');
    for (; *s; ++s) {
        if (*s == '"' || *s == '\\')
            putchar('\\');
        putchar(*s);
    }
    putchar('"');
}
int main(int argc, char **argv) {
    if (argc < 6 || argc > 9) {
        fprintf(stderr, "usage: render_ui_layout language page pairing mode control_granted [route [battery [hold_percent]]]\n");
        return 2;
    }
    ui_model m = {.language = atoi(argv[1]),
                  .page = atoi(argv[2]),
                  .pairing = atoi(argv[3]),
                  .mode = atoi(argv[4]),
                  .control_granted = atoi(argv[5]) != 0,
                  .usb_online = true,
                  .input_online = true,
                  .ble_ready = true,
                  .identity_ready = true,
                  .mcp_ready = true,
                  .http_ready = true,
                  .touch_ready = true,
                  .battery_percent = argc >= 8 ? atoi(argv[7]) : 75};
    strcpy(m.firmware_version,"0.9.10-ble-state");m.reset_reason=4;m.uptime_seconds=123;
    if(m.page==UI_CONNECT){
        m.free_internal_heap=2396;m.minimum_internal_heap=1084;m.largest_internal_block=512;
        m.previous_ap=(reset_trace_info){.valid=true,.stage=4,.error=0x101,.free_internal=2396,.largest_internal=512,.stack_free=768};
    }
    if (argc>=7 && atoi(argv[6])) {
        m.input_mode=INPUT_BLE; m.ble_connected=true; m.wifi_configured=true;
        m.wifi_connected=true; strcpy(m.wifi_address,"10.0.0.123"); m.usb_online=false;
    }
    if(argc>=7 && atoi(argv[6])==2){m.setup_ap=true;m.wifi_connected=false;strcpy(m.ap_name,"StopWatch-setup");strcpy(m.ap_password,"DEMO23456789");}
    if(argc>=7 && (atoi(argv[6])==3||atoi(argv[6])==4)){
        m.wifi_setup=true;m.setup_ap=false;m.wifi_setup_stage=4;
        m.wifi_setup_error=atoi(argv[6])==4?0x101:0;
        m.http_ready=false;m.http_error=0x101;
        m.free_internal_heap=2396;m.minimum_internal_heap=1084;
        m.largest_internal_block=512;m.free_psram=8121060;
    }
    if(argc>=7 && atoi(argv[6])>=5){m.input_online=false;m.ble_connected=atoi(argv[6])==6;}
    if(argc==9){m.hold_percent=atoi(argv[8]);m.pressed=m.page==UI_RESET?UI_RESET_HOLD:UI_BLE_RESET_HOLD;}
    strcpy(m.code, "123456");
    strcpy(m.fingerprint, "0123456789abcdef");
    ui_widget widgets[UI_WIDGET_LIMIT];
    size_t count = ui_layout(&m, widgets);
    putchar('[');
    for (size_t i = 0; i < count; ++i) {
        ui_widget *w = &widgets[i];
        if (i)
            putchar(',');
        printf("{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"scale\":%u,\"fg\":%u,\"bg\":%u,\"button\":%"
               "s,\"enabled\":%s,\"selected\":%s,\"action\":%d,\"text\":",
               w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->scale, w->foreground, w->background,
               w->button ? "true" : "false", w->enabled ? "true" : "false",
               w->selected ? "true" : "false", w->action);
        string(w->text);
        printf(",\"qr\":%s",w->qr?"true":"false");
        printf(",\"pressed\":%s,\"hold_percent\":%u",w->button&&w->action==m.pressed?"true":"false",
               w->button&&w->action==m.pressed?m.hold_percent:0);
        putchar('}');
    }
    puts("]");
    return 0;
}

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
    if (argc != 6 && argc != 7) {
        fprintf(stderr, "usage: render_ui_layout language page pairing mode control_granted\n");
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
                  .touch_ready = true};
    if (argc==7 && atoi(argv[6])) {
        m.input_mode=INPUT_BLE; m.ble_connected=true; m.wifi_configured=true;
        m.wifi_connected=true; strcpy(m.wifi_address,"10.0.0.123"); m.usb_online=false;
    }
    if(argc==7 && atoi(argv[6])==2){m.setup_ap=true;m.wifi_connected=false;strcpy(m.ap_name,"StopWatch-setup");strcpy(m.ap_password,"DEMO23456789");}
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
        putchar('}');
    }
    puts("]");
    return 0;
}

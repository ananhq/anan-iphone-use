#!/usr/bin/env python3
"""Predefined actual UI-painter acceptance; fake display/mutex, no device I/O."""
from pathlib import Path
import subprocess
import tempfile
import sys
root=Path(__file__).resolve().parents[2]
fixture=r'''
#include "ui.h"
#include "display.h"
#include "wifi_qr.h"
#include "avatar.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
static avatar_pose expected;static unsigned frames;
bool display_paint(display_strip_painter painter,void *ctx) {
    uint16_t actual[466*16],want[466*16];
    for(int y=0;y<466;y+=16) {
        painter(actual,y,ctx);avatar_paint_strip(&expected,want,y);
        assert(!memcmp(actual,want,sizeof(actual)));
    }
    frames++;return true;
}
bool display_paint_region(int x,int y,int end,int bottom,display_strip_painter painter,void*ctx){assert(x>=0&&y>=0&&end<=466&&bottom<=466);return display_paint(painter,ctx);}
bool display_paint_regions(const display_region *regions,unsigned count,display_region_painter painter,void *ctx){
    uint16_t actual[466*16],want[466*16];
    for(unsigned i=0;i<count;i++)for(int y=regions[i].y;y<regions[i].y+regions[i].h;){
        int rows=466*16/regions[i].w;if(rows>regions[i].y+regions[i].h-y)rows=regions[i].y+regions[i].h-y;
        painter(actual,regions[i].x,y,regions[i].w,rows,ctx);avatar_paint_region(&expected,want,regions[i].x,y,regions[i].w,rows);
        assert(!memcmp(actual,want,regions[i].w*rows*2));y+=rows;
    }
    if(count)frames++;return true;
}
uint8_t display_ascii_row(char c,unsigned row){(void)c;(void)row;assert(0);return 0;}
bool wifi_qr_make(wifi_qr*q,const char*s,const char*p){(void)q;(void)s;(void)p;assert(0);return false;}
unsigned wifi_qr_size(const wifi_qr*q){(void)q;return 0;}
bool wifi_qr_module(const wifi_qr*q,int x,int y){(void)q;(void)x;(void)y;return false;}
int main(void) {
    ui_init();avatar_state s;eyes_pose target={.eye={{161,233,52,72},{305,233,52,72}}};avatar_reset(&s,0);avatar_step(&s,0,&target,(tilt_gaze){0},&expected);
    for(int lang=0;lang<2;lang++) {
        if(lang)avatar_step(&s,17,&target,(tilt_gaze){0},&expected);
        ui_model m={.avatar_valid=true,.avatar=expected,.page=UI_HOME,.control_granted=true,.language=lang,.battery_percent=75,
            .input_busy=true,.pressed=UI_START,.hold_percent=100,.focus=UI_STOP};
        assert(ui_present(&m));ui_model shown;assert(ui_displayed(&shown)&&shown.control_granted);
    }
    assert(frames>=1);puts("Actual UI compact painter: both languages contain exactly the native avatar, no labels/QR/buttons: passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-eyes-render-') as work:
    work=Path(work);(work/'freertos').mkdir()
    (work/'freertos/FreeRTOS.h').write_text('#include <assert.h>\n#define configASSERT(x) assert(x)\n#define portMAX_DELAY 0\n')
    (work/'freertos/semphr.h').write_text('typedef int SemaphoreHandle_t;\nstatic inline int xSemaphoreCreateMutex(void){return 1;}\nstatic inline void xSemaphoreTake(int x,int y){(void)x;(void)y;}\nstatic inline void xSemaphoreGive(int x){(void)x;}\n')
    (work/'fixture.c').write_text(fixture)
    exe=work/'render'
    sys.path.insert(0,str(root/'tools'))
    from compile_avatar_host import compile_avatar
    compile_avatar(exe,[work/'fixture.c',root/'main/ui/ui.c',root/'main/ui/ui_layout.c'],[work])
    subprocess.run([str(exe)],check=True)

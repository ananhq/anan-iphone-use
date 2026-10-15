#!/usr/bin/env python3
"""Predefined actual cropped-DMA function check; SDK calls replaced, no board."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'main/hardware/display.c').read_text()
function=source[source.index('bool display_paint_region('):].split('// Compact avatar regions',1)[0]
fixture=r'''
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdatomic.h>
#include "display.h"
#define WIDTH 466
#define HEIGHT 466
#define STRIP 16
#define ESP_OK 0
#define ESP_ERR_TIMEOUT -1
#define pdTRUE 1
#define pdMS_TO_TICKS(x) (x)
#define SCREEN_DRAWING 0
#define SCREEN_READY 1
typedef int esp_err_t;
static bool available=true,timeout,send_error;
static uint16_t memory[466*16],*pixels=memory;
static int panel,transferred;static atomic_int stage;
static unsigned sends;static int expected_x,expected_end,expected_y,expected_bottom;
static bool failed(int e){(void)e;return false;}
static int xSemaphoreTake(int s,int ms){(void)s;assert(ms==500);return !timeout;}
static int esp_lcd_panel_draw_bitmap(int p,int x,int y,int end,int bottom,const void *data) {
    (void)p;assert(x==expected_x&&end==expected_end&&y==expected_y&&bottom<=expected_bottom);
    assert(bottom-y<=16);const uint16_t*values=data;
    for(int row=y;row<bottom;row++)for(int col=x;col<end;col++)assert(*values++==(uint16_t)(row*466+col));
    expected_y=bottom;sends++;return send_error?-2:0;
}
static void paint(uint16_t *out,int y,void*c){(void)c;for(int r=0;r<16;r++)for(int x=0;x<466;x++)out[r*466+x]=(uint16_t)((y+r)*466+x);}
'''
cases=r'''
int main(void) {
    expected_x=70;expected_end=400;expected_y=137;expected_bottom=180;
    assert(display_paint_region(70,137,400,180,paint,NULL)&&sends==3&&expected_y==180);
    unsigned before=sends;assert(!display_paint_region(-1,0,20,20,paint,NULL));assert(sends==before);
    expected_y=137;timeout=true;assert(!display_paint_region(70,137,400,180,paint,NULL));
    assert(!available);before=sends;timeout=false;assert(!display_paint_region(70,137,400,180,paint,NULL)&&sends==before);
    available=true;send_error=true;expected_y=137;assert(!display_paint_region(70,137,400,180,paint,NULL)&&!available);
    puts("Actual cropped DMA: stride/byte-preserving packing, partial strip, bounds, timeout/error and no unsafe reuse: passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-region-') as work:
    c=Path(work)/'check.c';c.write_text(fixture+function+cases);exe=Path(work)/'check'
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',*include_flags(),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)

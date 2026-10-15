#!/usr/bin/env python3
"""Predefined compact DMA + even-window regression. Actual function, fake SDK."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from firmware_paths import include_flags
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'main/hardware/display.c').read_text()
function=source.split('// Compact avatar regions',1)[1].split('\n',1)[1]
fixture=r'''
#include "display.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
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
static uint16_t memory[466*16+2],*pixels=memory+1;
static unsigned char coverage[466*466];
static bool available=true,timeout,send_error;
static int panel,transferred;static atomic_int stage;
static unsigned sends,drawn;
static bool failed(int code){(void)code;return false;}
static int xSemaphoreTake(int handle,int ms){(void)handle;assert(ms==500);return !timeout;}
static int esp_lcd_panel_draw_bitmap(int handle,int x,int y,int end,int bottom,const void *data){
 (void)handle;assert(x>=0&&end<=466&&y>=0&&bottom<=466&&(end-x)*(bottom-y)<=466*16);
 // CO5300 window contract: starts even, inclusive ends odd. Check each DMA,
 // not just the outer region: 80px width previously produced a 93-row batch.
 assert(!(x&1)&&!(y&1)&&!(end&1)&&!(bottom&1));
 const uint16_t *p=data;for(int r=y;r<bottom;r++)for(int c=x;c<end;c++)assert(*p++==(uint16_t)(r*466+c));
 for(int r=y;r<bottom;r++)for(int c=x;c<end;c++)coverage[r*466+c]++;
 sends++;drawn+=(end-x)*(bottom-y);assert(memory[0]==0x1234&&memory[466*16+1]==0x5678);return send_error?-2:0;
}
static void paint(uint16_t *out,int x,int y,int width,int rows,void *ctx){
 (void)ctx;for(int r=0;r<rows;r++)for(int c=0;c<width;c++)out[r*width+c]=(uint16_t)((y+r)*466+x+c);
}
'''
cases=r'''
static void single(display_region r){
 memset(coverage,0,sizeof(coverage));sends=drawn=0;
 assert(display_paint_regions(&r,1,paint,NULL));
 int left=r.x&~1,top=r.y&~1,end=(r.x+r.w+1)&~1,bottom=(r.y+r.h+1)&~1;
 assert(drawn==(unsigned)((end-left)*(bottom-top)));
 for(int y=0;y<466;y++)for(int x=0;x<466;x++)
   assert(coverage[y*466+x]==(x>=left&&x<end&&y>=top&&y<bottom));
}
int main(void){
 memory[0]=0x1234;memory[466*16+1]=0x5678;
 display_region narrow={80,100,80,180};assert(display_paint_regions(&narrow,1,paint,NULL));
 assert(sends==2&&drawn==80*180);display_paint_metrics m=display_last_paint_metrics();assert(m.transfers==2&&m.pixels==14400);
 display_region overlap[2]={{20,100,80,80},{70,120,80,80}};sends=drawn=0;
 assert(display_paint_regions(overlap,2,paint,NULL));assert(sends==2&&drawn==130*100);
 unsigned before=sends;display_region invalid={-2,0,20,20};assert(!display_paint_regions(&invalid,1,paint,NULL)&&sends==before);
 assert(display_paint_regions(NULL,0,paint,NULL)&&sends==before);
 // One-pixel moves, odd origins/sizes, all edges, and different DMA capacities.
 const int starts[]={0,1,2,231,232,463,464,465};
 const int sizes[]={1,2,3,79,80,81,129,130,465,466};
 for(unsigned i=0;i<sizeof(starts)/sizeof(*starts);i++)for(unsigned j=0;j<sizeof(sizes)/sizeof(*sizes);j++){
   int x=starts[i],n=sizes[j];if(x+n>466)continue;
   single((display_region){x,x,n,n});
   single((display_region){x,465,n,1});
   single((display_region){465,x,1,n});
 }
 single((display_region){0,0,466,466});
 unsigned seed=0x5300;for(unsigned i=0;i<160;i++){
   seed=1664525*seed+1013904223;int x=seed%466;
   seed=1664525*seed+1013904223;int y=seed%466;
   seed=1664525*seed+1013904223;int w=1+seed%(466-x);
   seed=1664525*seed+1013904223;int h=1+seed%(466-y);
   single((display_region){x,y,w,h});
 }
 timeout=true;assert(!display_paint_regions(&narrow,1,paint,NULL)&&!available);
 before=sends;timeout=false;assert(!display_paint_regions(&narrow,1,paint,NULL)&&sends==before);
 available=true;send_error=true;assert(!display_paint_regions(&narrow,1,paint,NULL)&&!available);
 puts("Actual compact DMA: even windows/batches, pixel coverage at edges/odd sizes, stride/capacity, overlap, metrics and fail-closed completion: passed");
}
'''
with tempfile.TemporaryDirectory(prefix='anan-compact-') as d:
 c=Path(d)/'check.c';c.write_text(fixture+function+cases);exe=Path(d)/'check'
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',*include_flags(),str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)

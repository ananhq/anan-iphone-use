#!/usr/bin/env python3
"""Render documentation previews from the firmware's layout and bitmap assets.
No device, phone simulator, HID, transport or E2E execution.
"""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[0]))
from firmware_paths import include_flags
import json
import io
import pathlib
import re
import subprocess
import sys
from PIL import Image, ImageDraw
root = pathlib.Path(__file__).resolve().parent.parent
out = pathlib.Path(sys.argv[1] if len(sys.argv)>1 else root/'build/ui-previews')
out.mkdir(parents=True,exist_ok=True)
exe = out/'layout-export'
subprocess.run(['cc','-std=c11',*include_flags(),str(root/'tools/render_ui_layout.c'),str(root/'main/ui/ui_layout.c'),'-o',str(exe)],check=True)
eyes_exe=out/'eyes-export'
from compile_avatar_host import compile_avatar
compile_avatar(eyes_exe,[root/'tools/render_eyes.c'])
font = {int(code,16):bytes(int(b,16) for b in data.split(',')) for code,data in re.findall(r'\{0x([0-9a-f]+), \{([^}]+)\}\}',(root/'main/ui/ui_font.h').read_text())}
src = (root/'main/hardware/display.c').read_text()
def rows(name,next_name):
    text=src.split('static const uint8_t '+name,1)[1].split(next_name,1)[0]
    return [[int(x) for x in match.split(',')] for match in re.findall(r'\{([0-9,\s]+)\}',text)]
letters=rows('letters','static const uint8_t digits')
digits=rows('digits','static uint8_t glyph')
def glyph(c):
    c=c.upper()
    if 'A'<=c<='Z':return letters[ord(c)-65]
    if '0'<=c<='9':return digits[ord(c)-48]
    return {':':[0,0,4,0,0,4,0],'.':[0,0,0,0,0,0,4],'/':[1,1,2,4,8,16,16],'-':[0,0,0,14,0,0,0],'+':[0,4,4,14,4,4,0],'?':[14,17,1,2,4,0,4],'%':[25,26,2,4,8,11,19]}.get(c,[0]*7)
def rgb(c):return ((c>>11)*255//31,((c>>5)&63)*255//63,(c&31)*255//31,255)
def cjk_size(scale):return 18 if scale==1 else 28 if scale>=3 else 24
scenes=[('home-mcp',0,0,0,0),('home-active',0,0,0,1),('settings',1,0,0,0),
        ('pair-start',2,0,1,0),('pair-confirm',2,1,1,0),('pair-working',2,2,1,0),('pair-ready',2,3,1,0),
        ('pair-expired',2,5,1,0),('reset',3,0,0,0),('reset-working',4,0,0,0)]
scenes += [('input-usb',5,0,0,0),('input-ble',5,0,0,0),('home-wireless',0,0,0,0),('pair-wireless',2,0,1,0),('pair-hotspot',2,0,1,0),('hotspot-ready',2,3,1,0)]
scenes += [('connect',6,0,0,0),('more',7,0,0,0),('language',8,0,0,0),('ble-reset',9,0,0,0),('ble-reset-hold',9,0,0,0),('home-battery-unknown',0,0,0,0),('home-battery-low',0,0,0,0)]
scenes += [('wifi-starting',2,0,1,0),('wifi-failed',2,0,1,0)]
scenes += [('home-ble-wait',0,0,0,0),('home-ble-linked',0,0,0,0)]
qr_exe=out/'qr-export'
subprocess.run(['cc','-std=c11','-D_POSIX_C_SOURCE=200809L',*include_flags(),str(root/'tools/wireless_acceptance/wifi-qr.c'),str(root/'main/network/wifi_qr.c'),str(root/'main/vendor/qrcodegen.c'),'-o',str(qr_exe)],check=True)
subprocess.run([str(qr_exe),str(out/'synthetic-wifi-qr.pgm')],check=True)
qr_image=Image.open(out/'synthetic-wifi-qr.pgm').convert('RGBA')
records=[]
for language in [0,1]:
 for name,page,pair,mode,control_granted in scenes:
    route=5 if name=='home-ble-wait' else 6 if name=='home-ble-linked' else 3 if name=='wifi-starting' else 4 if name=='wifi-failed' else 2 if name in ['pair-hotspot','hotspot-ready'] else int(name in ['input-ble','home-wireless','pair-wireless'])
    widgets=json.loads(subprocess.check_output([str(exe),str(language),str(page),str(pair),str(mode),str(control_granted),str(route),str(-1 if name=='home-battery-unknown' else 15 if name=='home-battery-low' else 75)]+(['50'] if name=='ble-reset-hold' else [])))
    image=Image.open(io.BytesIO(subprocess.check_output([str(eyes_exe),'0','0']))).convert('RGBA') if control_granted else Image.new('RGBA',(466,466),rgb(0x0882))
    draw=ImageDraw.Draw(image);pixels=image.load()
    for w in widgets:
        if w.get('qr'):
            image.paste(qr_image,((466-qr_image.width)//2,w['y']))
            continue
        if w['button']:
            box=(w['x'],w['y'],w['x']+w['w']-1,w['y']+w['h']-1)
            draw.rounded_rectangle(box,radius=14,fill=rgb(w['bg']),outline=rgb(0xffff if w['pressed'] else 0xfe89) if w['selected'] or w['pressed'] else None,width=2)
            if w['hold_percent']:
                left=w['x']+14;top=w['y']+w['h']-10;width=(w['w']-28)*w['hold_percent']//100
                draw.rectangle((left,top,left+width-1,top+3),fill=rgb(0xffff))
        scale=w['scale'];width=sum(6*scale if ord(c)<128 else cjk_size(scale) for c in w['text']);cursor=w['x']+(w['w']-width)//2
        for c in w['text']:
            size=7*scale if ord(c)<128 else cjk_size(scale);top=w['y']+(w['h']-size)//2
            if ord(c)<128:
                data=glyph(c)
                for row in range(7):
                 for col in range(5):
                  if data[row]&(1<<(4-col)):
                   draw.rectangle((cursor+col*scale,top+row*scale,cursor+(col+1)*scale-1,top+(row+1)*scale-1),fill=rgb(w['fg']))
                cursor+=6*scale
            else:
                data=font[ord(c)]
                for dy in range(size):
                 for dx in range(size):
                  row,col=dy*24//size,dx*24//size
                  if data[row*3+col//8]&(1<<(7-col%8)) and 0<=cursor+dx<466 and 0<=top+dy<466:pixels[cursor+dx,top+dy]=rgb(w['fg'])
                cursor+=size
    mask=Image.new('L',(466,466));ImageDraw.Draw(mask).ellipse((0,0,465,465),fill=255);image.putalpha(mask)
    filename=('zh' if language==0 else 'en')+'-'+name+'.png';image.save(out/filename)
    records.append({'file':filename,'widgets':widgets})
(out/'layouts.json').write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
gallery=Image.new('RGB',(4*486,((len(records)+3)//4)*516),(24,28,34));g=ImageDraw.Draw(gallery)
for i,record in enumerate(records):
    x=(i%4)*486+10;y=(i//4)*516+30;im=Image.open(out/record['file']);gallery.paste(im,(x,y),im);g.text((x,y-20),record['file'],fill='white')
gallery.save(out/'gallery.png')
print('Rendered %d static bilingual UI previews to %s'%(len(records),out))

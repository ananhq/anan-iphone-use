#!/usr/bin/env python3
"""Render documentation previews from the firmware's layout and bitmap assets.
No device, phone simulator, HID, transport or E2E execution.
"""
import json
import pathlib
import re
import subprocess
import sys
from PIL import Image, ImageDraw
root = pathlib.Path(__file__).resolve().parent.parent
out = pathlib.Path(sys.argv[1] if len(sys.argv)>1 else root/'build/ui-previews')
out.mkdir(parents=True,exist_ok=True)
exe = out/'layout-export'
subprocess.run(['cc','-std=c11','-I',str(root/'main'),str(root/'tools/render_ui_layout.c'),str(root/'main/ui_layout.c'),'-o',str(exe)],check=True)
font = {int(code,16):bytes(int(b,16) for b in data.split(',')) for code,data in re.findall(r'\{0x([0-9a-f]+), \{([^}]+)\}\}',(root/'main/ui_font.h').read_text())}
src = (root/'main/display.c').read_text()
def rows(name,next_name):
    text=src.split('static const uint8_t '+name,1)[1].split(next_name,1)[0]
    return [[int(x) for x in match.split(',')] for match in re.findall(r'\{([0-9,\s]+)\}',text)]
letters=rows('letters','static const uint8_t digits')
digits=rows('digits','static uint8_t glyph')
def glyph(c):
    c=c.upper()
    if 'A'<=c<='Z':return letters[ord(c)-65]
    if '0'<=c<='9':return digits[ord(c)-48]
    return {':':[0,0,4,0,0,4,0],'.':[0,0,0,0,0,0,4],'/':[1,1,2,4,8,16,16],'-':[0,0,0,14,0,0,0],'+':[0,4,4,14,4,4,0],'?':[14,17,1,2,4,0,4]}.get(c,[0]*7)
def rgb(c):return ((c>>11)*255//31,((c>>5)&63)*255//63,(c&31)*255//31,255)
def cjk_size(scale):return 18 if scale==1 else 28 if scale>=3 else 24
scenes=[('home-mcp',0,0,0,0),('home-active',0,0,0,1),('settings',1,0,0,0),
        ('pair-start',2,0,1,0),('pair-confirm',2,1,1,0),('pair-working',2,2,1,0),('pair-ready',2,3,1,0),
        ('pair-expired',2,5,1,0),('reset',3,0,0,0),('reset-working',4,0,0,0)]
scenes += [('input-usb',5,0,0,0),('input-ble',5,0,0,0),('home-wireless',0,0,0,0),('pair-wireless',2,0,1,0),('pair-hotspot',2,0,1,0),('hotspot-ready',2,3,1,0)]
qr_exe=out/'qr-export'
subprocess.run(['cc','-std=c11','-D_POSIX_C_SOURCE=200809L','-I',str(root/'main'),str(root/'tools/wireless_acceptance/wifi-qr.c'),str(root/'main/wifi_qr.c'),str(root/'main/vendor/qrcodegen.c'),'-o',str(qr_exe)],check=True)
subprocess.run([str(qr_exe),str(out/'synthetic-wifi-qr.pgm')],check=True)
qr_image=Image.open(out/'synthetic-wifi-qr.pgm').convert('RGBA')
records=[]
for language in [0,1]:
 for name,page,pair,mode,control_granted in scenes:
    widgets=json.loads(subprocess.check_output([str(exe),str(language),str(page),str(pair),str(mode),str(control_granted),str(2 if name in ['pair-hotspot','hotspot-ready'] else int(name in ['input-ble','home-wireless','pair-wireless']))]))
    image=Image.new('RGBA',(466,466),rgb(0x0882));draw=ImageDraw.Draw(image);pixels=image.load()
    for w in widgets:
        if w.get('qr'):
            image.paste(qr_image,((466-qr_image.width)//2,w['y']))
            continue
        if w['button']:
            box=(w['x'],w['y'],w['x']+w['w']-1,w['y']+w['h']-1)
            draw.rounded_rectangle(box,radius=14,fill=rgb(w['bg']),outline=rgb(0xfe89) if w['selected'] else None,width=2)
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

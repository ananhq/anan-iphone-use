#!/usr/bin/env python3
"""Generate only UI glyphs from the pinned OFL font; no device access."""
import argparse
import hashlib
import pathlib
import sys
parser = argparse.ArgumentParser()
parser.add_argument('font', type=pathlib.Path)
parser.add_argument('--output', type=pathlib.Path, default=pathlib.Path('main/ui/ui_font.h'))
args = parser.parse_args()
expected = 'a3041811a78c361b1de50f953c805e0244951c21c5bd412f7232ef0d899af0da'
if hashlib.sha256(args.font.read_bytes()).hexdigest() != expected:
    sys.exit('Font source hash differs from the pinned Noto Sans SC input')
from PIL import Image, ImageDraw, ImageFont
font = ImageFont.truetype(str(args.font), 22)
font.set_variation_by_axes([500])
source = pathlib.Path('main/ui/ui_layout.c').read_text()
characters = sorted({c for c in source if ord(c) > 127}, key=ord)
lines = ['// SPDX-License-Identifier: OFL-1.1',
 '// Copyright 2014-2021 Adobe, Reserved Font Name Source.',
 '// Generated StopWatch UI Glyphs, not the full Noto font; see licenses/NotoSansSC-OFL-1.1.txt.',
 '// Source SHA-256: '+expected, '#pragma once', '#include <stdint.h>',
 'typedef struct { uint32_t code; uint8_t rows[72]; } ui_font_glyph;',
 'static const ui_font_glyph ui_font[] = {']
for c in characters:
    image = Image.new('L', (24, 24))
    bbox = font.getbbox(c)
    x = (24-(bbox[2]-bbox[0]))//2-bbox[0]
    y = (24-(bbox[3]-bbox[1]))//2-bbox[1]
    ImageDraw.Draw(image).text((x,y), c, font=font, fill=255)
    pixels = image.load()
    data = []
    for y in range(24):
        for byte in range(3):
            data.append(sum((1 << (7-bit)) if pixels[8*byte+bit,y] >= 96 else 0 for bit in range(8)))
    if not any(data):
        sys.exit('Empty glyph for '+c)
    lines.append('    {0x%x, {%s}}, // %s' % (ord(c), ','.join('0x%02x'%b for b in data),c))
lines += ['};', '#define UI_FONT_COUNT (sizeof(ui_font) / sizeof(ui_font[0]))', '']
args.output.write_text('\n'.join(lines))
print('Generated %d CJK UI glyphs (%d bitmap bytes)'%(len(characters),len(characters)*72))

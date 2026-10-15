#!/usr/bin/env python3
"""Predefined target-compiler UI stack-budget check; no board or simulator I/O.

Measures frames with the candidate's compiler/options and linked libc. The known
formatting path plus 2 KiB reserve must fit each UI task. This is a static budget
guard, not a measured runtime high-water mark or exhaustive worst-case proof.
"""
import json
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
build = Path(sys.argv[1]).resolve()
commands = json.loads((build / 'compile_commands.json').read_text())
frames = {}
with tempfile.TemporaryDirectory(prefix='anan-ui-stack-') as work:
    for entry in commands:
        if Path(entry['file']).name not in ('device.c', 'ui.c', 'ui_layout.c','display.c','avatar.cpp','avatar_native.cpp','avatar_activity.cpp','avatar_engine.cpp'):
            continue
        args = shlex.split(entry['command'])
        compiler = args[0]
        output = Path(work) / Path(entry['file']).stem
        for flag, suffix in (('-o', '.o'), ('-MF', '.d')):
            if flag in args:
                args[args.index(flag) + 1] = str(output) + suffix
        subprocess.run(args + ['-fstack-usage'], cwd=entry['directory'], check=True)
        for line in output.with_suffix('.su').read_text().splitlines():
            location, size, category = line.split('\t')
            assert category == 'static', (location.rsplit(':', 1)[-1], category)
            # C++ function names include scope separators and parameter names.
            name=location.split(':',3)[-1]
            frames[name] = int(size)

objdump = compiler.removesuffix('gcc').removesuffix('g++') + 'objdump'
for function in ('snprintf', '_svfprintf_r', '__ssprint_r'):
    assembly = subprocess.check_output([objdump, '-D', '--disassemble=' + function,
                                       str(build / 'anan_stopwatch.elf')], text=True)
    match = re.search(r'\bentry\s+a1,\s*(0x[\da-f]+|\d+)', assembly)
    assert match, 'Missing linked libc frame: ' + function
    frames[function] = int(match[1], 0)

source = (root / 'main/app/device.c').read_text()
constants = {k: int(v) for k, v in re.findall(r'^#define\s+(\w+)\s+(\d+)\s*$', source, re.M)}
def allocation(function):
    token = re.search(r'xTaskCreate\(' + function + r',\s*"[^"]+",\s*(\w+)', source)[1]
    return int(token) if token.isdigit() else constants[token]

formatting = ['ui_layout', 'row', 'button', 'snprintf', '_svfprintf_r', '__ssprint_r']
paths = {'touch_input': ['touch_input', 'ui_hit'] + formatting,
         'buttons': ['buttons', 'ui_next_focus'] + formatting,
         'screen': ['screen', 'ui_present'] + formatting}
checks = {}
for task, path in paths.items():
    known = sum(frames[name] for name in path)
    allocated = allocation(task)
    checks[task] = {'allocatedBytes': allocated, 'knownPathFramesBytes': known,
                    'additionalReserveBytes': 2048, 'passes': allocated >= known + 2048}
inlined_or_absent=[]
def cpp_frame(prefix):
    matches=[size for name,size in frames.items() if name.startswith(prefix)]
    if not matches:
        # At -O2 an inlined helper has no separate frame; its storage is included
        # in the caller's measured frame. Retain this fact in the report.
        inlined_or_absent.append(prefix)
    return max(matches,default=0)
avatar_render=frames['screen']+sum(cpp_frame(p) for p in (
    'void avatar_step(', 'void AvatarEngine::update(', 'void AvatarEngine::render(',
    'void AvatarEngine::drawEye(', 'AvatarEngine::drawEye(',
    'void AvatarDisplay::fillTriangle(', 'void {anonymous}::push('))
avatar_reset=frames['screen']+cpp_frame('void avatar_reset(')+cpp_frame('bool AvatarEngine::begin(')
for name,known in (('screen-avatar-render',avatar_render),('screen-avatar-reset',avatar_reset)):
    allocated=allocation('screen')
    checks[name]={'allocatedBytes':allocated,'knownPathFramesBytes':known,
                  'additionalReserveBytes':2048,'passes':allocated>=known+2048}
activity_render=frames['screen']+sum(cpp_frame(p) for p in (
    'void avatar_step(', 'void AvatarEngine::update(', 'void AvatarEngine::render(',
    'void AvatarDisplay::endWrite(', 'void avatar_activity_draw(',
    'void {anonymous}::stroke(', 'void AvatarDisplay::drawWideLine(',
    'void AvatarDisplay::fillTriangle(', 'void {anonymous}::push('))
activity_sample=frames['screen']+cpp_frame('void avatar_step(')+cpp_frame('AvatarActivityFrame AvatarActivity::sample(')
for name,known in (('screen-activity-render',activity_render),('screen-activity-sample',activity_sample)):
    allocated=allocation('screen')
    checks[name]={'allocatedBytes':allocated,'knownPathFramesBytes':known,
                  'additionalReserveBytes':2048,'passes':allocated>=known+2048}
compact_paint=frames['screen']+frames['ui_present']+frames['display_paint_regions']+frames['paint_avatar_region']+cpp_frame('void avatar_paint_region(')+cpp_frame('void {anonymous}::span(')+cpp_frame('void {anonymous}::triangle_row(')
checks['screen-compact-paint']={'allocatedBytes':allocation('screen'),'knownPathFramesBytes':compact_paint,
    'additionalReserveBytes':2048,'passes':allocation('screen')>=compact_paint+2048}
print(json.dumps({'framesBytes': frames, 'tasks': checks,
                  'optimizedHelpersWithoutSeparateFrame':sorted(set(inlined_or_absent)),
                  'deviceVerified': False}, indent=2))
assert all(c['passes'] for c in checks.values()), 'UI task stack budget too small'

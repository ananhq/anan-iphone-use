#!/usr/bin/env python3
"""Compile the actual native avatar for offline fixtures/document previews."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[0]))
from firmware_paths import include_flags
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
def compile_avatar(output,c_sources,includes=()):
    output=Path(output)
    with tempfile.TemporaryDirectory(prefix='anan-avatar-objects-') as d:
        objects=[];headers=[*include_flags()]
        for p in includes:headers+=['-I',str(p)]
        for i,source in enumerate([*c_sources,ROOT/'main/ui/eyes.c']):
            obj=Path(d)/f'{i}.o'
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',*headers,'-c',str(source),'-o',str(obj)],check=True);objects.append(str(obj))
        subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror',*headers,
                        str(ROOT/'main/ui/avatar.cpp'),str(ROOT/'main/ui/avatar_native.cpp'),str(ROOT/'main/ui/avatar_activity.cpp'),
                        str(ROOT/'main/vendor/stopwatch_avatar/avatar_engine.cpp'),
                        *objects,'-o',str(output)],check=True)

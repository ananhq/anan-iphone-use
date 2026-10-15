#!/usr/bin/env python3
"""Predefined byte-for-byte renderer comparison; reference saved before edits.

Usage: capture <reference-dir>, then compare <same-dir>. No device/model I/O.
The reference executable and hashes are ignored local artifacts, not a portable
cross-compiler rendering promise.
"""
from pathlib import Path
import hashlib
import json
import subprocess
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from compile_avatar_host import compile_avatar, ROOT

mode=sys.argv[1];out=Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
assert mode in ('capture','compare')
exe=out/('reference' if mode=='capture' else 'candidate')
compile_avatar(exe,[ROOT/'tools/render_eyes.c'])
samples=[(m,t) for m in range(11) for t in (0,33,100,231,600,1000,1500,3000,6000,12500)]
hashes={f'{m}:{t}':hashlib.sha256(subprocess.check_output([str(exe),str(m),str(t)])).hexdigest()
        for m,t in samples}
manifest=out/'reference.json'
if mode=='capture':
    assert not manifest.exists(),'Use a new directory; do not overwrite the pre-edit reference'
    manifest.write_text(json.dumps(hashes,indent=2)+'\n')
else:
    previous=json.loads(manifest.read_text())
    assert hashes==previous,[k for k in hashes if hashes[k]!=previous.get(k)]
    print(f'Original/new native full RGB frames identical for {len(samples)} predefined samples')

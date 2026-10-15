#!/usr/bin/env python3
"""Deterministic project source offer. Explicit public build-input allowlist only."""
import gzip
import io
from pathlib import Path
import sys
import tarfile

ROOT=Path(__file__).resolve().parents[1]
ROOT_FILES=('CMakeLists.txt','sdkconfig.defaults','dependencies.lock','partitions.csv',
            'build.sh','package.py','flash.sh','flash.py','LICENSE','NOTICE',
            'THIRD_PARTY.md','THIRD_PARTY.zh-CN.md','SOURCE.md','SOURCE.zh-CN.md')
TOOLS=('source_bundle.py','generate_ui_font.py')

def inputs():
    paths=[ROOT/p for p in ROOT_FILES]+[ROOT/'tools'/p for p in TOOLS]
    paths += [p for p in (ROOT/'main').rglob('*') if p.is_file()
              and (p.suffix in ('.c','.h','.cpp','.hpp','.md','.yml','.html') or p.name=='CMakeLists.txt')
              and not any(part.startswith('.') for part in p.relative_to(ROOT/'main').parts)]
    paths += [p for p in (ROOT/'licenses').iterdir() if p.is_file() and p.suffix=='.txt']
    return sorted(set(paths))

def bundle():
    raw=io.BytesIO()
    with tarfile.open(fileobj=raw,mode='w',format=tarfile.USTAR_FORMAT) as archive:
        for path in inputs():
            if path.is_symlink():raise ValueError('Symlink in source allowlist')
            data=path.read_bytes();name='anan-iphone-use/'+path.relative_to(ROOT).as_posix()
            entry=tarfile.TarInfo(name);entry.size=len(data);entry.mtime=0
            entry.mode=0o755 if path.suffix=='.sh' else 0o644
            archive.addfile(entry,io.BytesIO(data))
    return gzip.compress(raw.getvalue(),compresslevel=9,mtime=0)

if __name__=='__main__':
    output=Path(sys.argv[1]);data=bundle();output.parent.mkdir(parents=True,exist_ok=True)
    if not output.exists() or output.read_bytes()!=data:output.write_bytes(data)
    print('Project source archive:',len(data),'bytes')

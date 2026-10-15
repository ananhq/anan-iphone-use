#!/usr/bin/env python3
"""Defined before source bundling/HTTP implementation. No network or device I/O."""
from pathlib import Path
import hashlib
import io
import subprocess
import tarfile
import tempfile
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='anan-source-') as d:
    a=Path(d)/'a.tar.gz';b=Path(d)/'b.tar.gz'
    for p in (a,b):subprocess.run(['python3',str(root/'tools/source_bundle.py'),str(p)],check=True)
    assert a.read_bytes()==b.read_bytes()
    with tarfile.open(fileobj=io.BytesIO(a.read_bytes())) as archive:
        files={m.name:archive.extractfile(m).read() for m in archive if m.isfile()}
    for name in ('main/ui/avatar.cpp','main/vendor/stopwatch_avatar/avatar_engine.cpp','LICENSE','NOTICE','licenses/StopWatch-avatar-AGPL-3.0.txt','dependencies.lock','sdkconfig.defaults','tools/source_bundle.py','SOURCE.md','SOURCE.zh-CN.md'):
        assert files['anan-iphone-use/'+name]==(root/name).read_bytes()
    for name,data in files.items():
        assert not name.startswith('/') and '..' not in name.split('/')
        assert not any(x in name.split('/') for x in ('.git','build','artifacts','managed_components','__pycache__'))
        assert name.rsplit('/',1)[-1] not in ('sdkconfig',)
        assert b'/Users/' not in data and b'/private/tmp/' not in data
    mcp=(root/'main/network/mcp.c').read_text()
    assert mcp.count('"/source", HTTP_GET, source_handler')==2
    assert 'href="/source"' in (root/'main/web/setup.html').read_text()
    print('Deterministic allowlisted source archive, no private paths, HTTP/HTTPS source routes:',hashlib.sha256(a.read_bytes()).hexdigest())

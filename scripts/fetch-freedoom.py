#!/usr/bin/env python3
"""Fetch pinned freely distributable game data and verify the IWAD's SHA256."""
from pathlib import Path
import hashlib,io,urllib.request,zipfile
root=Path(__file__).resolve().parents[1];dest=root/'assets/freedoom';dest.mkdir(parents=True,exist_ok=True)
expected='7323bcc168c5a45ff10749b339960e98314740a734c30d4b9f3337001f9e703d'
wad=dest/'freedoom1.wad'
if wad.exists() and hashlib.sha256(wad.read_bytes()).hexdigest()==expected:
 print('Freedoom IWAD verified (cached)');raise SystemExit(0)
url='https://github.com/freedoom/freedoom/releases/download/v0.13.0/freedoom-0.13.0.zip'
with urllib.request.urlopen(url,timeout=60) as r:
 data=r.read(64*1024*1024+1)
 if len(data)>64*1024*1024:raise RuntimeError('Freedoom archive exceeds size limit')
with zipfile.ZipFile(io.BytesIO(data)) as z:
 contents=z.read('freedoom-0.13.0/freedoom1.wad')
 if hashlib.sha256(contents).hexdigest()!=expected:raise RuntimeError('Freedoom IWAD checksum mismatch')
 for name in ['COPYING.txt','CREDITS.txt']:(dest/name).write_bytes(z.read('freedoom-0.13.0/'+name))
 tmp=wad.with_suffix('.wad.tmp');tmp.write_bytes(contents);tmp.replace(wad)
print('Downloaded and verified Freedoom Phase 1 v0.13.0')

#!/usr/bin/env python3
"""Archive tracked project and pinned engine sources, excluding local state."""
from pathlib import Path
import os, subprocess, zipfile, hashlib
root=Path(__file__).resolve().parents[1]
version=os.environ.get('VERSION','0.4.0-beta.1')
out=root/'dist';out.mkdir(exist_ok=True)
archive=out/f'maschine-mk1-revive-v{version}-source.zip'
prefix=f'maschine-mk1-revive-v{version}/'
def tracked(base):
 return subprocess.check_output(['git','-C',str(base),'ls-files','-z']).decode().split('\0')
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
 for name in tracked(root):
  if name and (root/name).is_file():z.write(root/name,prefix+name)
 engine=root/'third_party/doomgeneric'
 for name in tracked(engine):
  if name and (engine/name).is_file():z.write(engine/name,prefix+'third_party/doomgeneric/'+name)
with (out/'SHA256SUMS').open('w') as f:
 for path in sorted(out.glob(f'*v{version}*.zip')):
  f.write(hashlib.sha256(path.read_bytes()).hexdigest()+'  '+path.name+'\n')
print(archive)

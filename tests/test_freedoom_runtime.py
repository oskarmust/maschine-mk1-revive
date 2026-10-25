"""Smoke-test the real engine + WAD through its shared framebuffer and input."""
from pathlib import Path
import mmap, os, struct, subprocess, tempfile, time
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='mk1-freedoom-test-') as tmp:
 path=Path(tmp)/'framebuffer'
 path.write_bytes(bytes(56+2*255*64))
 with path.open('r+b') as f:
  mm=mmap.mmap(f.fileno(),0)
  struct.pack_into('IIII',mm,0,0x4d4b3145,os.getpid(),0,0)
  env=dict(os.environ,MK1_DOOM_SHARED=str(path))
  with (Path(tmp)/'engine.log').open('wb') as log:
   proc=subprocess.Popen([str(root/'build/Debug/mk1-freedoom'),'-iwad',str(root/'assets/freedoom/freedoom1.wad'),'-warp','1','1','-skill','2','-nosound','-nomusic','-config',str(Path(tmp)/'default.cfg')],env=env,stdout=log,stderr=log,cwd=tmp)
   try:
    deadline=time.monotonic()+30
    while struct.unpack_from('I',mm,8)[0]<100 and time.monotonic()<deadline:
     if proc.poll() is not None: raise RuntimeError((Path(tmp)/'engine.log').read_text())
     time.sleep(.05)
    seq=struct.unpack_from('I',mm,8)[0]
    assert seq>=100, (Path(tmp)/'engine.log').read_text()
    before=bytes(mm[56:56+255*64]);assert max(before)<=31 and len(set(before))>8
    health,armor,ammo,kills,total,episode,level,cards=struct.unpack_from('8i',mm,24)
    assert health==100 and ammo==50 and episode==1 and level==1 and total>0
    hud=bytes(mm[56+255*64:]);assert max(hud)<=31 and len(set(hud))>8
    Path('/tmp/mk1-native-hud.pgm').write_bytes(b'P5\n255 64\n255\n'+bytes(v*255//31 for v in hud))
    before_position=struct.unpack_from('ii',mm,16)
    struct.pack_into('I',mm,12,1<<13) # Held pad 14: walk forward.
    time.sleep(.7)
    struct.pack_into('I',mm,12,0)
    after=bytes(mm[56:56+255*64]);assert before!=after
    assert before_position!=struct.unpack_from('ii',mm,16), "Player did not move"
    Path('/tmp/mk1-real-freedoom.pgm').write_bytes(b'P5\n255 64\n255\n'+bytes(v*255//31 for v in after))
    struct.pack_into('I',mm,12,1<<11) # Held pad 12: fire.
    time.sleep(.8)
    struct.pack_into('I',mm,12,0)
    assert struct.unpack_from('i',mm,32)[0]<ammo, "Firing did not consume ammo"
    print('PASS: real Freedoom E1M1, grayscale frames, movement, firing and native status-bar framebuffer; sequence',seq)
   finally:
    proc.terminate();proc.wait(timeout=5)
  mm.close()

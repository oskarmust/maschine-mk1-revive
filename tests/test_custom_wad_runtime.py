"""Run the production launcher with custom IWAD+PWAD, verifying map and real frames."""
from pathlib import Path
import tempfile,subprocess,shutil,struct,plistlib,os
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='mk1-custom-wad-') as folder:
 tmp=Path(folder);macos=tmp/'Test.app/Contents/MacOS';resources=tmp/'Test.app/Contents/Resources/freedoom'
 macos.mkdir(parents=True);resources.mkdir(parents=True)
 shutil.copy2(root/'build/Debug/mk1-freedoom',macos/'mk1-freedoom')
 shutil.copy2(root/'assets/freedoom/freedoom1.wad',resources/'freedoom1.wad')
 base=tmp/'custom base with spaces.wad';shutil.copy2(resources/'freedoom1.wad',base)
 data=base.read_bytes();_,count,offset=struct.unpack_from('<4sII',data)
 lumps=[struct.unpack_from('<II8s',data,offset+i*16) for i in range(count)]
 first=next(i for i,x in enumerate(lumps) if x[2].rstrip(b'\0')==b'E1M1')
 chosen=lumps[first:first+11]
 things_at,things_size,_=chosen[1]
 spawn=next(struct.unpack_from('<hhhhh',data,at)[:2] for at in range(things_at,things_at+things_size,10) if struct.unpack_from('<hhhhh',data,at)[3]==1)
 payload=bytearray(b'PWAD'+bytes(8));directory=[]
 for i,(start,size,name) in enumerate(chosen):
  directory.append((len(payload),size,b'E1M2\0\0\0\0' if i==0 else name));payload.extend(data[start:start+size])
 offset=len(payload)
 for entry in directory:payload.extend(struct.pack('<II8s',*entry))
 struct.pack_into('<II',payload,4,len(directory),offset)
 patch=tmp/'custom patch with spaces.wad';patch.write_bytes(payload)
 config=tmp/'doom.plist';config.write_bytes(plistlib.dumps({'baseWAD':str(base),'patchWAD':str(patch)}))
 probe=tmp/'probe.c';probe.write_text('''
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <spawn.h>
#include <sys/stat.h>
#include <assert.h>
extern char **environ;
#define BLOG(...) ((void)0)
#include "mk1-bridge/freedoom_client.h"
int main(void){
 assert(start_freedoom());uint8_t frame[10880];bool ready=false;
 for(int i=0;i<300;i++){
  if(freedoom_frame(frame,0,0)&&atomic_load(&g_freedoom_shared->health)==100){ready=true;break;}
  usleep(50000);
 }
 assert(ready&&g_freedoom_pid>0);
 assert(atomic_load(&g_freedoom_shared->episode)==1&&atomic_load(&g_freedoom_shared->map)==2);
 uint8_t hud[10880];assert(freedoom_hud_frame(hud));assert(memcmp(frame,hud,sizeof(hud)));
 stop_freedoom();assert(!g_freedoom_pid&&!g_freedoom_shared);
 puts("PASS: production launcher loads custom IWAD and PWAD via argv; starts E1M2; real scene/HUD frames; engine cleanup");
}
''')
 text=probe.read_text().replace(' stop_freedoom();assert',f' assert(atomic_load(&g_freedoom_shared->player_x)=={spawn[0]*65536} && atomic_load(&g_freedoom_shared->player_y)=={spawn[1]*65536});\n stop_freedoom();assert')
 probe.write_text(text)
 subprocess.run(['clang','-O1','-I'+str(root),'-framework','CoreFoundation',str(probe),'-o',str(macos/'probe')],check=True)
 env=dict(os.environ,MK1_DOOM_CONFIG_PATH=str(config),MK1_DOOM_DATA_DIR=str(tmp))
 try:subprocess.run([str(macos/'probe')],cwd=tmp,env=env,check=True,timeout=30)
 except Exception:
  print((tmp/'freedoom.log').read_text() if (tmp/'freedoom.log').exists() else 'No engine log');raise
 log=(tmp/'freedoom.log').read_text();assert str(patch) in log,'PWAD not loaded'
 # Run Swift's picker validator against identical vectors, including malformed input.
 swift=tmp/'check.swift';swift.write_text('''import Foundation
@main struct Check {
 static func main() throws {
  let base=try DoomWADSettings.inspect(URL(fileURLWithPath:CommandLine.arguments[1]))
  let patch=try DoomWADSettings.inspect(URL(fileURLWithPath:CommandLine.arguments[2]))
  precondition(base.kind=="IWAD" && patch.kind=="PWAD" && base.commercial==patch.commercial)
  do { _ = try DoomWADSettings.inspect(URL(fileURLWithPath:CommandLine.arguments[3]));fatalError("Accepted bad WAD") } catch {}
  print("PASS: Swift picker validates same IWAD/PWAD and rejects malformed input")
 }
}
''')
 bad=tmp/'invalid.wad';bad.write_bytes(b'PWAD'+bytes(8))
 subprocess.run(['swiftc','-parse-as-library',str(root/'mk1-menubar/DoomWADSettings.swift'),str(swift),'-o',str(tmp/'swift-check')],check=True)
 subprocess.run([str(tmp/'swift-check'),str(base),str(patch),str(bad)],check=True)

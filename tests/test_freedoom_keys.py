"""Exercise the actual backend key queue for each physical pad and rotary pulse."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'mk1-freedoom/doomgeneric_mk1.c').read_text()
f=s[s.index('int DG_GetKey('):s.index('void DG_SetWindowTitle(')]
pre='''#include <assert.h>
#include <stdio.h>
#include "protocol.h"
#include "doomkeys.h"
#include "idle_pad_layout.h"
static mk1_doom_shared_t *shared;
'''
main='''
int main(void) {
 mk1_doom_shared_t state={0};shared=&state;
 struct {uint32_t mask;unsigned char key;} cases[]={
 {1u<<13,KEY_UPARROW},{1u<<9,KEY_DOWNARROW},
 {1u<<8,KEY_LEFTARROW},{1u<<10,KEY_RIGHTARROW},
 {1u<<11,KEY_FIRE},{1u<<15,KEY_USE},
 {1u<<12,KEY_STRAFE_L},{1u<<14,KEY_STRAFE_R},
 {1u<<4,KEY_ENTER},{1u<<5,KEY_TAB},
 {1u<<0,'1'},{1u<<1,'2'},{1u<<2,'3'},{1u<<3,'4'},
 {MK1_DOOM_FORWARD,KEY_UPARROW},{MK1_DOOM_BACKWARD,KEY_DOWNARROW},
 {MK1_DOOM_TURN_LEFT,KEY_LEFTARROW},{MK1_DOOM_TURN_RIGHT,KEY_RIGHTARROW},
 {MK1_DOOM_FIRE,KEY_FIRE}};
 for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++){
  int pressed;unsigned char key;
  atomic_store(&state.keys,cases[i].mask);
  assert(DG_GetKey(&pressed,&key)&&pressed&&key==cases[i].key);
  assert(!DG_GetKey(&pressed,&key));
  atomic_store(&state.keys,0);
  assert(DG_GetKey(&pressed,&key)&&!pressed&&key==cases[i].key);
  assert(!DG_GetKey(&pressed,&key));
 }
 struct {uint32_t raw;unsigned char key;} captured[]={
 {1,KEY_UPARROW},{5,KEY_DOWNARROW},{4,KEY_LEFTARROW},{6,KEY_RIGHTARROW},
 {7,KEY_FIRE},{3,KEY_USE},{0,KEY_STRAFE_L},{2,KEY_STRAFE_R}};
 for(unsigned i=0;i<sizeof(captured)/sizeof(captured[0]);i++){
  int pressed;unsigned char key;
  atomic_store(&state.keys,1u<<idle_pad_physical_index(captured[i].raw));
  assert(DG_GetKey(&pressed,&key)&&pressed&&key==captured[i].key);
  atomic_store(&state.keys,0);
  assert(DG_GetKey(&pressed,&key)&&!pressed&&key==captured[i].key);
 }
 puts("PASS: captured USB IDs -> physical WASD/action pads -> Doom key press/release");
}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'keys.c').write_text(pre+f+main)
 subprocess.run(['clang','-O2','-Wall','-Wextra','-I'+str(root/'mk1-freedoom'),'-I'+str(root/'mk1-bridge'),'-I'+str(root/'third_party/doomgeneric/doomgeneric'),str(p/'keys.c'),'-o',str(p/'keys')],check=True)
 subprocess.run([str(p/'keys')],check=True)

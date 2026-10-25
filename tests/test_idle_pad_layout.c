#include "../mk1-bridge/idle_pad_layout.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 const unsigned captured[]={12,13,14,15,8,9,10,11,4,5,6,7,0,1,2,3};
 for(unsigned physical=0;physical<16;physical++)
  assert(idle_pad_physical_index(captured[physical])==physical);
 assert(idle_pad_physical_index(16)==16);
 assert(idle_pad_physical_index(0xffffffffu)==16);
 puts("PASS: all 16 printed pad numbers match measured USB IDs");
}

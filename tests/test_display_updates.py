from pathlib import Path
s=Path('mk1-bridge/main.c').read_text()
f=s[s.index('static void forward_display(bridge_t *br, const uint8_t *raw_msg, size_t raw_len)\n{'):]
f=f[:f.index('\n// ---------------------------------------------------------------------------')]
prefix='''#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#define IPC_DISPLAY_HDR_LEN 20
#define DISPLAY_ROWS 64
#define DISPLAY_BYTES_PER_ROW 170
#define DLOG(...) ((void)0)
#define TLOG(...) ((void)0)
typedef struct { void *usb; } bridge_t;
static uint8_t g_display_fb[2][10880];
static uint64_t g_display_batch_start_ns[2];
static uint16_t g_display_row_start[2],g_display_row_end[2],g_display_byte_x_start[2],g_display_byte_x_end[2];
static bool g_display_have_region[2];
static uint32_t g_display_dirty_mask;
static pthread_mutex_t g_display_lock=PTHREAD_MUTEX_INITIALIZER;
static void stop_idle_screen(void) {}
static uint64_t timing_now_ns(void) {return 1;}
'''
test='''
static void update(bridge_t *br,uint16_t x,uint16_t y,uint8_t value) {
 uint8_t msg[24]={0}; uint16_t h=1,w=6; uint32_t n=4;
 memcpy(msg+8,&y,2);memcpy(msg+10,&x,2);memcpy(msg+12,&h,2);memcpy(msg+14,&w,2);memcpy(msg+16,&n,4);
 memset(msg+20,value,4);forward_display(br,msg,sizeof(msg));
}
int main(void) {
 bridge_t br={.usb=(void*)1};
 update(&br,30,20,0xaa); update(&br,0,2,0xbb);
 assert(g_display_row_start[0]==2 && g_display_row_end[0]==20);
 assert(g_display_byte_x_start[0]==0 && g_display_byte_x_end[0]==23);
 assert(g_display_fb[0][20*170+20]==0xaa && g_display_fb[0][2*170]==0xbb);
 update(&br,30,20,0); assert(g_display_fb[0][20*170+20]==0);
 // A black-only batch must still mark a region dirty and erase old content.
 g_display_have_region[0]=false; g_display_dirty_mask=0;
 update(&br,0,2,0);
 assert(g_display_fb[0][2*170]==0 && g_display_have_region[0] && g_display_dirty_mask==1);
 puts("PASS: disjoint updates retained; black clears applied and queued");
}
'''
Path('/tmp/mk1-display-regression.c').write_text(prefix+f+test)

import subprocess
subprocess.run(["clang", "-Wall", "-Wextra", "/tmp/mk1-display-regression.c", "-o", "/tmp/mk1-display-regression"], check=True)
subprocess.run(["/tmp/mk1-display-regression"], check=True)

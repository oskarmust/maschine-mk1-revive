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
#include "../mk1-bridge/freedoom_client.h"
int main(void) {
 for(int round=0;round<2;round++){
  assert(start_freedoom());pid_t pid=g_freedoom_pid;
  uint8_t fb[10880];bool ready=false;
  for(int i=0;i<300;i++){
   if(freedoom_frame(fb,0,0)&&atomic_load(&g_freedoom_shared->health)==100){ready=true;break;}
   usleep(50000);
  }
  assert(ready&&g_freedoom_pid>0);
  uint8_t hud[10880];assert(freedoom_hud_frame(hud));
  assert(memcmp(fb,hud,sizeof(hud))!=0);
  char path[PATH_MAX];snprintf(path,sizeof(path),"%s",g_freedoom_path);
  stop_freedoom();assert(!g_freedoom_shared&&g_freedoom_pid==0&&!g_freedoom_attempted);
  assert(kill(pid,0)<0&&errno==ESRCH);assert(access(path,F_OK)<0);
 }
 puts("PASS: production client starts, reads real frames, stops/reaps engine and cleans mmap; twice");
}

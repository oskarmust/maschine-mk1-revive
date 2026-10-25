/* MK1 display backend for doomgeneric (GPL-2.0-or-later). */
#include "doomgeneric.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "d_items.h"
#include "r_main.h"
#include "protocol.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <string.h>
static mk1_doom_shared_t *shared;
static uint64_t start_ms;
static unsigned char luminance[256];
static uint64_t milliseconds(void) {
 struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);
 return (uint64_t)ts.tv_sec*1000+ts.tv_nsec/1000000;
}
static void terminate(int sig){(void)sig;_exit(0);}
void DG_Init(void) {
 const char *path=getenv("MK1_DOOM_SHARED");
 if(!path){fprintf(stderr,"Missing MK1_DOOM_SHARED\n");exit(1);}
 int fd=open(path,O_RDWR);
 if(fd<0){perror("open shared framebuffer");exit(1);}
 shared=mmap(NULL,sizeof(*shared),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);close(fd);
 if(shared==MAP_FAILED||shared->magic!=MK1_DOOM_MAGIC){fprintf(stderr,"Bad framebuffer\n");exit(1);}
 start_ms=milliseconds();
 for(int i=0;i<256;i++)luminance[i]=(unsigned char)lroundf(sqrtf(i/255.f)*31);
 signal(SIGTERM,terminate);signal(SIGINT,terminate);
}
static void sample_region(uint8_t *destination,int source_y,int source_height)
{
 for(int y=0;y<MK1_DOOM_H;y++)for(int x=0;x<MK1_DOOM_W;x++) {
  int x0=x*DOOMGENERIC_RESX/MK1_DOOM_W,x1=(x+1)*DOOMGENERIC_RESX/MK1_DOOM_W;
  int y0=source_y+y*source_height/MK1_DOOM_H;
  int y1=source_y+(y+1)*source_height/MK1_DOOM_H;
  if(y1<=y0)y1=y0+1;
  unsigned total=0,count=0;
  for(int sy=y0;sy<y1;sy++)for(int sx=x0;sx<x1;sx++) {
   uint32_t rgb=DG_ScreenBuffer[sy*DOOMGENERIC_RESX+sx];
   total+=(((rgb>>16)&255)*77+((rgb>>8)&255)*150+(rgb&255)*29)>>8;count++;
  }
  destination[y*MK1_DOOM_W+x]=luminance[total/(count?count:1)];
 }
}
void DG_DrawFrame(void) {
 if(getppid()!=(pid_t)shared->owner_pid)_exit(0);
 if(players[consoleplayer].mo){
  atomic_store(&shared->player_x,players[consoleplayer].mo->x);
  atomic_store(&shared->player_y,players[consoleplayer].mo->y);
  player_t *p=&players[consoleplayer];
  atomic_store(&shared->health,p->health);
  atomic_store(&shared->armor,p->armorpoints);
  ammotype_t ammo=weaponinfo[p->readyweapon].ammo;
  atomic_store(&shared->ammo,ammo==am_noammo?-1:p->ammo[ammo]);
  atomic_store(&shared->kills,p->killcount);
  atomic_store(&shared->total_kills,totalkills);
  atomic_store(&shared->episode,gameepisode);atomic_store(&shared->map,gamemap);
  int cards=0;for(int i=0;i<NUMCARDS;i++)if(p->cards[i])cards|=1<<i;
  atomic_store(&shared->cards,cards);
 }
 atomic_fetch_add_explicit(&shared->sequence,1,memory_order_acq_rel);
 /* View size 10 renders a 320x168 playfield and the native 320x32 status bar.
  * Separate them before scaling, so the left LCD contains no HUD pixels. */
 sample_region(shared->pixels,0,gamestate==GS_LEVEL?168:200);
 if(gamestate==GS_LEVEL)sample_region(shared->hud_pixels,168,32);
 atomic_fetch_add_explicit(&shared->sequence,1,memory_order_release);
}
void DG_SleepMs(uint32_t ms){usleep(ms*1000);}
uint32_t DG_GetTicksMs(void){return (uint32_t)(milliseconds()-start_ms);}
int DG_GetKey(int *pressed,unsigned char *key) {
 static uint32_t previous;
 uint32_t bits=atomic_load_explicit(&shared->keys,memory_order_acquire),wanted=0;
 const unsigned char keys[]={KEY_DOWNARROW,KEY_UPARROW,KEY_STRAFE_L,KEY_STRAFE_R,
  KEY_FIRE,KEY_USE,KEY_LEFTARROW,KEY_RIGHTARROW,KEY_ENTER,KEY_TAB,'1','2','3','4'};
 /* Physical WASD cluster: 14=W, 9=A, 10=S, 11=D. */
 if(bits&((1u<<9)|MK1_DOOM_BACKWARD))wanted|=1u<<0;
 if(bits&((1u<<13)|MK1_DOOM_FORWARD))wanted|=1u<<1;
 if(bits&(1u<<12))wanted|=1u<<2;
 if(bits&(1u<<14))wanted|=1u<<3;
 if(bits&((1u<<11)|MK1_DOOM_FIRE))wanted|=1u<<4;
 if(bits&(1u<<15))wanted|=1u<<5;
 if(bits&((1u<<8)|MK1_DOOM_TURN_LEFT))wanted|=1u<<6;
 if(bits&((1u<<10)|MK1_DOOM_TURN_RIGHT))wanted|=1u<<7;
 if(bits&(1u<<4))wanted|=1u<<8; /* Pad 5: menu/respawn confirm. */
 if(bits&(1u<<5))wanted|=1u<<9; /* Pad 6: automap. */
 for(int i=0;i<4;i++)if(bits&(1u<<i))wanted|=1u<<(10+i);
 uint32_t changed=previous^wanted;
 if(!changed)return 0;
 unsigned index=(unsigned)__builtin_ctz(changed);
 *pressed=(wanted&(1u<<index))!=0;*key=keys[index];previous^=1u<<index;
 return 1;
}
void DG_SetWindowTitle(const char *title){fprintf(stderr,"MK1 Freedoom: %s\n",title);}
int main(int argc,char **argv) {
 doomgeneric_Create(argc,argv);
 R_SetViewSize(10,0); /* Native status bar is extracted onto the right LCD. */
 while(1)doomgeneric_Tick();
}

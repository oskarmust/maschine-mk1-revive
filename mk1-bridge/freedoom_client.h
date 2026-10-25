#ifndef MK1_FREEDOOM_CLIENT_H
#define MK1_FREEDOOM_CLIENT_H
#include "../mk1-freedoom/protocol.h"
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <errno.h>
static mk1_doom_shared_t *g_freedoom_shared;
static pid_t g_freedoom_pid;
static char g_freedoom_path[PATH_MAX];
static bool g_freedoom_attempted;
static bool g_freedoom_have_frame;
static uint8_t g_freedoom_last_frame[10880];
static uint8_t g_freedoom_last_hud[10880];
static uint64_t g_freedoom_turn_until, g_freedoom_move_until, g_freedoom_fire_until;
static uint32_t g_freedoom_turn_key, g_freedoom_move_key;

static void stop_freedoom(void)
{
    if (g_freedoom_pid > 0) {
        kill(g_freedoom_pid,SIGTERM);
        /* The backend's signal handler exits immediately; reap the child. */
        while(waitpid(g_freedoom_pid,NULL,0)<0&&errno==EINTR){}
        g_freedoom_pid=0;
    }
    if(g_freedoom_shared){munmap(g_freedoom_shared,sizeof(*g_freedoom_shared));g_freedoom_shared=NULL;}
    if(g_freedoom_path[0]){unlink(g_freedoom_path);g_freedoom_path[0]=0;}
    g_freedoom_attempted=false;g_freedoom_have_frame=false;
    g_freedoom_turn_until=g_freedoom_move_until=g_freedoom_fire_until=0;
}
static bool start_freedoom(void)
{
    stop_freedoom();g_freedoom_attempted=true;
    char exe[PATH_MAX],helper[PATH_MAX],wad[PATH_MAX],config[PATH_MAX];
    uint32_t size=sizeof(exe);
    if(_NSGetExecutablePath(exe,&size)!=0)return false;
    char *slash=strrchr(exe,'/');if(!slash)return false;*slash=0;
    snprintf(helper,sizeof(helper),"%s/mk1-freedoom",exe);
    snprintf(wad,sizeof(wad),"%s/../Resources/freedoom/freedoom1.wad",exe);
    if(access(helper,X_OK)||access(wad,R_OK)){BLOG("Freedoom: missing helper or WAD");return false;}
    snprintf(g_freedoom_path,sizeof(g_freedoom_path),"/tmp/mk1-freedoom-XXXXXX");
    int fd=mkstemp(g_freedoom_path);
    if(fd<0)return false;
    fcntl(fd,F_SETFD,FD_CLOEXEC);
    if(ftruncate(fd,sizeof(mk1_doom_shared_t))!=0){close(fd);unlink(g_freedoom_path);return false;}
    void *map=mmap(NULL,sizeof(mk1_doom_shared_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    close(fd);if(map==MAP_FAILED){unlink(g_freedoom_path);return false;}
    g_freedoom_shared=map;memset(map,0,sizeof(*g_freedoom_shared));
    g_freedoom_shared->magic=MK1_DOOM_MAGIC;g_freedoom_shared->owner_pid=(uint32_t)getpid();
    const char *home=getenv("HOME");
    if(home){
        snprintf(config,sizeof(config),"%s/Library/Application Support/MK1 Revive",home);mkdir(config,0700);
        snprintf(config,sizeof(config),"%s/Library/Application Support/MK1 Revive/Freedoom",home);mkdir(config,0700);
        snprintf(config,sizeof(config),"%s/Library/Application Support/MK1 Revive/Freedoom/default.cfg",home);
    } else snprintf(config,sizeof(config),"/tmp/mk1-freedoom-default.cfg");
    /* Explicit child environment; no changes to the bridge's process environment. */
    size_t nenv=0;while(environ[nenv])nenv++;
    char **env=calloc(nenv+2,sizeof(char*));
    if(!env){stop_freedoom();g_freedoom_attempted=true;return false;}
    size_t n=0;for(size_t i=0;i<nenv;i++)if(strncmp(environ[i],"MK1_DOOM_SHARED=",16)!=0)env[n++]=environ[i];
    char envpath[PATH_MAX+32];snprintf(envpath,sizeof(envpath),"MK1_DOOM_SHARED=%s",g_freedoom_path);env[n]=envpath;
    char *argv[]={helper,"-iwad",wad,"-warp","1","1","-skill","2","-nosound","-nomusic","-config",config,NULL};
    posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions,STDOUT_FILENO,"/tmp/maschine-mk1-revive/freedoom.log",O_WRONLY|O_CREAT|O_TRUNC,0600);
    posix_spawn_file_actions_adddup2(&actions,STDOUT_FILENO,STDERR_FILENO);
    int err=posix_spawn(&g_freedoom_pid,helper,&actions,NULL,argv,env);
    posix_spawn_file_actions_destroy(&actions);free(env);
    if(err){g_freedoom_pid=0;stop_freedoom();g_freedoom_attempted=true;BLOG("Freedoom spawn failed: %d",err);return false;}
    BLOG("Freedoom: real Doom engine started pid=%d, Freedoom 0.13.0 Phase 1 E1M1",g_freedoom_pid);
    return true;
}
static void freedoom_encoder(uint32_t index,float delta,uint64_t now)
{
    if(!isfinite(delta)||delta==0)return;
    if(index==3){g_freedoom_turn_key=delta>0?MK1_DOOM_TURN_RIGHT:MK1_DOOM_TURN_LEFT;g_freedoom_turn_until=now+100000000;}
    if(index==6){g_freedoom_move_key=delta>0?MK1_DOOM_FORWARD:MK1_DOOM_BACKWARD;g_freedoom_move_until=now+120000000;}
}
static bool freedoom_frame(uint8_t *fb,uint32_t pads,uint64_t now)
{
    if(!g_freedoom_shared)return false;
    if(g_freedoom_pid>0&&waitpid(g_freedoom_pid,NULL,WNOHANG)==g_freedoom_pid){
        g_freedoom_pid=0;BLOG("Freedoom: engine exited; see freedoom.log");return false;
    }
    uint32_t keys=pads;
    if(now<g_freedoom_turn_until)keys|=g_freedoom_turn_key;
    if(now<g_freedoom_move_until)keys|=g_freedoom_move_key;
    if(now<g_freedoom_fire_until)keys|=MK1_DOOM_FIRE;
    atomic_store_explicit(&g_freedoom_shared->keys,keys,memory_order_release);
    uint8_t pixels[2][MK1_DOOM_W*MK1_DOOM_H];
    for(int retry=0;retry<3;retry++){
        uint32_t before=atomic_load_explicit(&g_freedoom_shared->sequence,memory_order_acquire);
        if(!before||(before&1))continue;
        memcpy(pixels[0],g_freedoom_shared->pixels,sizeof(pixels[0]));
        memcpy(pixels[1],g_freedoom_shared->hud_pixels,sizeof(pixels[1]));
        atomic_thread_fence(memory_order_acquire);
        if(before!=atomic_load_explicit(&g_freedoom_shared->sequence,memory_order_acquire))continue;
        for(int display=0;display<2;display++){
            uint8_t *out=display==0?g_freedoom_last_frame:g_freedoom_last_hud;
            for(int y=0;y<64;y++)for(int x=0;x<85;x++){
                uint8_t a=pixels[display][y*255+x*3],b=pixels[display][y*255+x*3+1],c=pixels[display][y*255+x*3+2];
                out[y*170+x*2]=(uint8_t)((a<<3)|(b>>2));out[y*170+x*2+1]=(uint8_t)((b<<6)|c);
            }
        }
        memcpy(fb,g_freedoom_last_frame,sizeof(g_freedoom_last_frame));
        g_freedoom_have_frame=true;
        return true;
    }
    if(g_freedoom_have_frame){memcpy(fb,g_freedoom_last_frame,sizeof(g_freedoom_last_frame));return true;}
    return false;
}
static bool freedoom_hud_frame(uint8_t *fb)
{
    if(!g_freedoom_have_frame)return false;
    memcpy(fb,g_freedoom_last_hud,sizeof(g_freedoom_last_hud));
    return true;
}
#endif

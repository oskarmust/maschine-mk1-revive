#ifndef MK1_STUDIO_SERVICE_H
#define MK1_STUDIO_SERVICE_H
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
static int g_studio_socket=-1;
static dispatch_source_t g_studio_reader,g_studio_timer;
static char g_studio_socket_path[PATH_MAX],g_studio_state_path[PATH_MAX];
static uint64_t g_studio_watch_until;
static bool g_studio_profile_error;
static bool studio_reload(void){
 studio_profile_t next;if(!studio_read_profile(&next)){g_studio_profile_error=true;return false;}
 pthread_mutex_lock(&g_pad_state_lock);
 for(int i=0;i<16;i++)if(g_sent_pressure[i]||g_pending_hit_off[i]){
  pthread_mutex_unlock(&g_pad_state_lock);g_studio_profile_error=true;
  BLOG("Studio: release pads before applying profile");return false;
 }
 g_studio_profile=next;memset(g_prev_pressure,0,sizeof(g_prev_pressure));
 memset(g_above_threshold,0,sizeof(g_above_threshold));
 atomic_store(&g_idle_pad_mask,0);
 pthread_mutex_unlock(&g_pad_state_lock);g_studio_profile_error=false;
 BLOG("Studio: input profile applied");return true;
}
static void studio_publish(void){
 if(timing_now_ns()>g_studio_watch_until)return;
 char text[8192];size_t n=0;
 #define STATE(...) do{int wrote=snprintf(text+n,sizeof(text)-n,__VA_ARGS__);if(wrote>0)n+=(size_t)wrote;}while(0)
 uint64_t now=timing_now_ns();
 STATE("{\"pid\":%d,\"usb\":%s,\"maschine\":%s,\"idle\":%s,\"menu\":%s,\"game\":%d,\"profileError\":%s,\"buttons\":%llu,\"on\":%d,\"off\":%d,\"pressure\":%d,\"pads\":[",getpid(),g_bridge.usb?"true":"false",g_bridge.srv&&mk1_server_is_input_ready(g_bridge.srv)?"true":"false",atomic_load(&g_idle_active)?"true":"false",g_idle_games.menu?"true":"false",g_idle_games.selected,g_studio_profile_error?"true":"false",(unsigned long long)atomic_load(&g_studio_buttons),g_pad_hit_on_threshold,g_pad_hit_off_threshold,g_pad_pressure_threshold);
 for(int i=0;i<16;i++)STATE("%s%u",i?",":"",atomic_load(&g_studio_pressures[i]));
 STATE("],\"encoderDelta\":[");for(int i=0;i<11;i++)STATE("%s%d",i?",":"",atomic_load(&g_studio_encoder_delta[i]));
 STATE("],\"encoderAge\":[");for(int i=0;i<11;i++){uint64_t t=atomic_load(&g_studio_encoder_time[i]);STATE("%s%llu",i?",":"",(unsigned long long)(t?(now-t)/1000000:999999));}
 STATE("],\"doomError\":%s,\"pongAI\":[%s,%s]}\n",g_freedoom_error[0]?"true":"false",g_idle_games.pong.left_idle>=10?"true":"false",g_idle_games.pong.right_manual?"false":"true");
 #undef STATE
 if(n>=sizeof(text))return;
 char tmp[PATH_MAX];snprintf(tmp,sizeof(tmp),"%s.tmp",g_studio_state_path);
 int fd=open(tmp,O_WRONLY|O_CREAT|O_TRUNC,0600);if(fd<0)return;
 if(write(fd,text,n)==(ssize_t)n)rename(tmp,g_studio_state_path);
 close(fd);
}
static void studio_start(void){
 const char *home=getenv("HOME");if(!home)return;
 char dir[PATH_MAX];snprintf(dir,sizeof(dir),"%s/Library/Application Support/MK1 Revive",home);mkdir(dir,0700);
 snprintf(g_studio_socket_path,sizeof(g_studio_socket_path),"%s/control.sock",dir);
 snprintf(g_studio_state_path,sizeof(g_studio_state_path),"%s/state.json",dir);
 snprintf(g_studio_profile_path,sizeof(g_studio_profile_path),"%s/controls.plist",dir);
 studio_profile_defaults(&g_studio_profile);studio_reload();
 struct sockaddr_un addr={0};addr.sun_family=AF_UNIX;
 if(strlen(g_studio_socket_path)>=sizeof(addr.sun_path)){BLOG("Studio socket path too long");return;}
 snprintf(addr.sun_path,sizeof(addr.sun_path),"%s",g_studio_socket_path);
 g_studio_socket=socket(AF_UNIX,SOCK_DGRAM,0);if(g_studio_socket<0)return;
 fcntl(g_studio_socket,F_SETFL,O_NONBLOCK);fcntl(g_studio_socket,F_SETFD,FD_CLOEXEC);
 unlink(g_studio_socket_path);
 if(bind(g_studio_socket,(struct sockaddr*)&addr,sizeof(addr))<0){close(g_studio_socket);g_studio_socket=-1;return;}
 chmod(g_studio_socket_path,0600);
 g_studio_reader=dispatch_source_create(DISPATCH_SOURCE_TYPE_READ,g_studio_socket,0,dispatch_get_main_queue());
 dispatch_source_set_event_handler(g_studio_reader,^{
  char command[128];ssize_t count;
  while((count=recv(g_studio_socket,command,sizeof(command)-1,0))>0){
   command[count]=0;
   if(!strncmp(command,"WATCH",5)){g_studio_watch_until=timing_now_ns()+2500000000ULL;studio_publish();}
   else if(!strncmp(command,"RELOAD",6))studio_reload();
   else if(atomic_load(&g_idle_active)){
    int game=-1;
    if(sscanf(command,"PLAY %d",&game)==1&&game>=0&&game<3){stop_freedoom();g_idle_games.selected=game;idle_games_start(&g_idle_games);}
    else if(!strncmp(command,"MENU",4)){g_idle_games.menu=true;}
   }
  }
 });dispatch_resume(g_studio_reader);
 g_studio_timer=dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,dispatch_get_main_queue());
 dispatch_source_set_timer(g_studio_timer,dispatch_time(DISPATCH_TIME_NOW,100*NSEC_PER_MSEC),100*NSEC_PER_MSEC,10*NSEC_PER_MSEC);
 dispatch_source_set_event_handler(g_studio_timer,^{studio_publish();});dispatch_resume(g_studio_timer);
 BLOG("Studio: private control socket ready");
}
static void studio_stop(void){
 if(g_studio_reader){dispatch_source_cancel(g_studio_reader);g_studio_reader=NULL;}
 if(g_studio_timer){dispatch_source_cancel(g_studio_timer);g_studio_timer=NULL;}
 if(g_studio_socket>=0){close(g_studio_socket);g_studio_socket=-1;unlink(g_studio_socket_path);}
 unlink(g_studio_state_path);
}
#endif

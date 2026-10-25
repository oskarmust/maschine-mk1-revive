#ifndef MK1_INPUT_PROFILE_H
#define MK1_INPUT_PROFILE_H
#include <stdatomic.h>
#include <sys/stat.h>
typedef struct {bool override,enabled;int on,off,pressure,gain,target;} studio_pad_t;
typedef struct {bool enabled,invert;int gain,target;} studio_encoder_t;
typedef struct {studio_pad_t pads[16];studio_encoder_t encoders[11];} studio_profile_t;
static studio_profile_t g_studio_profile;
static pthread_mutex_t g_pad_state_lock=PTHREAD_MUTEX_INITIALIZER;
static _Atomic unsigned g_studio_pressures[16];
static _Atomic int g_studio_encoder_delta[11];
static _Atomic uint64_t g_studio_encoder_time[11];
static _Atomic uint64_t g_studio_buttons;
static char g_studio_profile_path[PATH_MAX];
static void studio_profile_defaults(studio_profile_t *p){
 memset(p,0,sizeof(*p));
 for(int i=0;i<16;i++)p->pads[i]=(studio_pad_t){.enabled=true,.on=300,.off=150,.pressure=200,.gain=100,.target=i};
 for(int i=0;i<11;i++)p->encoders[i]=(studio_encoder_t){.enabled=true,.gain=100,.target=i};
}
static int studio_int(CFDictionaryRef d,CFStringRef key,int fallback){
 CFTypeRef v=CFDictionaryGetValue(d,key);int n=fallback;
 if(v&&CFGetTypeID(v)==CFNumberGetTypeID())CFNumberGetValue(v,kCFNumberIntType,&n);return n;
}
static bool studio_bool(CFDictionaryRef d,CFStringRef key,bool fallback){
 CFTypeRef v=CFDictionaryGetValue(d,key);
 return v&&CFGetTypeID(v)==CFBooleanGetTypeID()?CFBooleanGetValue(v):fallback;
}
static bool studio_read_profile(studio_profile_t *p){
 studio_profile_defaults(p);
 if(!g_studio_profile_path[0]||access(g_studio_profile_path,F_OK))return true;
 FILE *f=fopen(g_studio_profile_path,"rb");if(!f)return false;
 uint8_t bytes[65536];size_t n=fread(bytes,1,sizeof(bytes),f);fclose(f);
 if(n==sizeof(bytes))return false;
 CFDataRef data=CFDataCreate(NULL,bytes,n);
 CFPropertyListRef obj=CFPropertyListCreateWithData(NULL,data,kCFPropertyListImmutable,NULL,NULL);CFRelease(data);
 if(!obj||CFGetTypeID(obj)!=CFDictionaryGetTypeID()){if(obj)CFRelease(obj);return false;}
 bool valid=true;
 CFArrayRef pads=CFDictionaryGetValue(obj,CFSTR("pads"));
 if(pads&&(CFGetTypeID(pads)!=CFArrayGetTypeID()||CFArrayGetCount(pads)!=16))valid=false;
 if(pads&&CFGetTypeID(pads)==CFArrayGetTypeID())for(int i=0;i<16&&i<CFArrayGetCount(pads);i++){
  CFDictionaryRef d=CFArrayGetValueAtIndex(pads,i);if(CFGetTypeID(d)!=CFDictionaryGetTypeID()){valid=false;break;}
  studio_pad_t *q=&p->pads[i];q->override=studio_bool(d,CFSTR("override"),false);q->enabled=studio_bool(d,CFSTR("enabled"),true);
  q->on=studio_int(d,CFSTR("on"),300);q->off=studio_int(d,CFSTR("off"),150);q->pressure=studio_int(d,CFSTR("pressure"),200);
  q->gain=studio_int(d,CFSTR("gain"),100);q->target=studio_int(d,CFSTR("target"),i);
  if(q->on<1||q->on>4095||q->off<0||q->off>=q->on||q->pressure<1||q->pressure>4095||q->gain<10||q->gain>400||q->target<0||q->target>15)valid=false;
 }
 CFArrayRef enc=CFDictionaryGetValue(obj,CFSTR("encoders"));
 if(enc&&(CFGetTypeID(enc)!=CFArrayGetTypeID()||CFArrayGetCount(enc)!=11))valid=false;
 if(enc&&CFGetTypeID(enc)==CFArrayGetTypeID())for(int i=0;i<11&&i<CFArrayGetCount(enc);i++){
  CFDictionaryRef d=CFArrayGetValueAtIndex(enc,i);if(CFGetTypeID(d)!=CFDictionaryGetTypeID()){valid=false;break;}
  studio_encoder_t *q=&p->encoders[i];q->enabled=studio_bool(d,CFSTR("enabled"),true);q->invert=studio_bool(d,CFSTR("invert"),false);
  q->gain=studio_int(d,CFSTR("gain"),100);q->target=studio_int(d,CFSTR("target"),i);
  if(q->gain<10||q->gain>400||q->target<0||q->target>10)valid=false;
 }
 CFRelease(obj);return valid;
}
static unsigned studio_pressure(unsigned raw,studio_pad_t q){
 if(!q.enabled)return 0;unsigned value=raw*(unsigned)q.gain/100;return value>4095?4095:value;
}
#endif

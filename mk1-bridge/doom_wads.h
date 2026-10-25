#ifndef MK1_DOOM_WADS_H
#define MK1_DOOM_WADS_H
#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

typedef struct { int kind,episode,map; bool commercial; } doom_wad_info;
static uint32_t doom_le32(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static bool doom_wad_validate(const char *path,int expected,doom_wad_info *info,char *error,size_t cap){
 FILE *f=fopen(path,"rb");if(!f){snprintf(error,cap,"Cannot read selected WAD");return false;}
 struct stat st;unsigned char header[12];bool ok=false;
 memset(info,0,sizeof(*info));info->episode=1;info->map=1;
 if(fstat(fileno(f),&st)||!S_ISREG(st.st_mode)||st.st_size<12||st.st_size>256LL*1024*1024||fread(header,1,12,f)!=12)goto invalid;
 info->kind=!memcmp(header,"IWAD",4)?1:(!memcmp(header,"PWAD",4)?2:0);
 if(!info->kind||(expected&&info->kind!=expected))goto invalid;
 uint32_t count=doom_le32(header+4),offset=doom_le32(header+8);
 if(!count||count>262144||offset<12||(uint64_t)offset+(uint64_t)count*16>(uint64_t)st.st_size||fseek(f,offset,SEEK_SET))goto invalid;
 bool map_found=false;
 for(uint32_t i=0;i<count;i++){
  unsigned char entry[16];if(fread(entry,1,16,f)!=16)goto invalid;
  if((uint64_t)doom_le32(entry)+(uint64_t)doom_le32(entry+4)>(uint64_t)st.st_size)goto invalid;
  const unsigned char *n=entry+8;
  if(!map_found&&n[0]=='E'&&n[1]>='1'&&n[1]<='9'&&n[2]=='M'&&n[3]>='1'&&n[3]<='9'&&!n[4]){info->episode=n[1]-'0';info->map=n[3]-'0';map_found=true;}
  if(!map_found&&!memcmp(n,"MAP",3)&&n[3]>='0'&&n[3]<='9'&&n[4]>='0'&&n[4]<='9'&&!n[5]){info->map=(n[3]-'0')*10+n[4]-'0';info->commercial=true;map_found=info->map>0;}
 }
 if(info->kind==1&&!map_found)goto invalid;
 // Mapless PWADs (textures/sounds) are allowed; episode==0 means use base map.
 if(info->kind==2&&!map_found)info->episode=0;
 ok=true;goto done;
 invalid:snprintf(error,cap,"Invalid or unsupported WAD (classic IWAD/PWAD, max 256 MiB)");
 done:fclose(f);return ok;
}
static bool doom_config_string(CFDictionaryRef dict,const char *key,char *path,size_t cap){
 CFStringRef name=CFStringCreateWithCString(NULL,key,kCFStringEncodingUTF8);
 CFTypeRef value=CFDictionaryGetValue(dict,name);CFRelease(name);
 if(!value)return true;
 return CFGetTypeID(value)==CFStringGetTypeID()&&CFStringGetCString(value,path,(CFIndex)cap,kCFStringEncodingUTF8)&&path[0]=='/';
}
static bool doom_wad_settings(const char *settings,char *base,size_t cap,char *patch,doom_wad_info *start,char *error,size_t errorcap){
 patch[0]=0;
 if(access(settings,F_OK)==0){
  CFURLRef url=CFURLCreateFromFileSystemRepresentation(NULL,(const UInt8*)settings,(CFIndex)strlen(settings),false);
  CFReadStreamRef stream=url?CFReadStreamCreateWithFile(NULL,url):NULL;if(url)CFRelease(url);
  CFPropertyListRef plist=NULL;
  if(stream&&CFReadStreamOpen(stream))plist=CFPropertyListCreateWithStream(NULL,stream,0,kCFPropertyListImmutable,NULL,NULL);
  if(stream){CFReadStreamClose(stream);CFRelease(stream);}
  bool valid=plist&&CFGetTypeID(plist)==CFDictionaryGetTypeID()&&doom_config_string((CFDictionaryRef)plist,"baseWAD",base,cap)&&doom_config_string((CFDictionaryRef)plist,"patchWAD",patch,cap);
  if(plist)CFRelease(plist);
  if(!valid){snprintf(error,errorcap,"Invalid Doom WAD settings");return false;}
 }
 if(!doom_wad_validate(base,1,start,error,errorcap))return false;
 if(patch[0]){
  doom_wad_info addition;if(!doom_wad_validate(patch,2,&addition,error,errorcap))return false;
  if(addition.episode){
   if(addition.commercial!=start->commercial){snprintf(error,errorcap,"PWAD map format does not match base IWAD");return false;}
   start->episode=addition.episode;start->map=addition.map;
  }
 }
 return true;
}
#endif

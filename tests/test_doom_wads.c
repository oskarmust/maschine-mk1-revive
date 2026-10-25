#include <assert.h>
#include "../mk1-bridge/doom_wads.h"
static void put32(unsigned char *p,uint32_t v){for(int i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static void fixture(const char *path,const char *kind,const char *map){
 unsigned char data[28]={0};memcpy(data,kind,4);put32(data+4,1);put32(data+8,12);memcpy(data+20,map,strlen(map));
 FILE *f=fopen(path,"wb");assert(f);assert(fwrite(data,1,28,f)==28);fclose(f);
}
int main(void){
 char dir[]="/tmp/mk1-wad-tests-XXXXXX";assert(mkdtemp(dir));
 char base[PATH_MAX],patch[PATH_MAX],settings[PATH_MAX],error[160];doom_wad_info info;
 snprintf(base,sizeof(base),"%s/base with spaces.wad",dir);fixture(base,"IWAD","E1M1");
 assert(doom_wad_validate(base,1,&info,error,sizeof(error))&&!info.commercial&&info.episode==1&&info.map==1);
 snprintf(patch,sizeof(patch),"%s/add-on.wad",dir);fixture(patch,"PWAD","E1M2");
 assert(doom_wad_validate(patch,2,&info,error,sizeof(error))&&info.map==2);
 fixture(patch,"PWAD","MAP01");assert(doom_wad_validate(patch,2,&info,error,sizeof(error))&&info.commercial);
 snprintf(settings,sizeof(settings),"%s/config.plist",dir);
 FILE *f=fopen(settings,"w");fprintf(f,"<?xml version=\"1.0\"?><plist version=\"1.0\"><dict><key>patchWAD</key><string>%s</string></dict></plist>",patch);fclose(f);
 char selected[PATH_MAX],addon[PATH_MAX];snprintf(selected,sizeof(selected),"%s",base);
 assert(!doom_wad_settings(settings,selected,sizeof(selected),addon,&info,error,sizeof(error))); // Doom II patch vs Doom I base
 fixture(patch,"PWAD","E1M2");assert(doom_wad_settings(settings,selected,sizeof(selected),addon,&info,error,sizeof(error))&&info.map==2);
 fixture(patch,"PWAD","TEXTURE1");assert(doom_wad_validate(patch,2,&info,error,sizeof(error))&&info.episode==0);
 assert(!doom_wad_validate(patch,1,&info,error,sizeof(error)));
 f=fopen(patch,"wb");fwrite("PWAD",1,4,f);fclose(f);assert(!doom_wad_validate(patch,2,&info,error,sizeof(error)));
 fixture(patch,"PWAD","E1M1");f=fopen(patch,"r+b");fseek(f,8,SEEK_SET);unsigned char bad[4]={255,255,255,255};fwrite(bad,1,4,f);fclose(f);assert(!doom_wad_validate(patch,2,&info,error,sizeof(error)));
 unlink(base);unlink(patch);unlink(settings);rmdir(dir);
 puts("PASS: WAD header/directory bounds, IWAD/PWAD roles, spaced paths, map selection and incompatible base rejection");
}

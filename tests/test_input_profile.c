#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include "../mk1-bridge/input_profile.h"
int main(void){
 studio_profile_t p;studio_profile_defaults(&p);
 for(int i=0;i<16;i++)assert(p.pads[i].target==i&&p.pads[i].enabled&&p.pads[i].gain==100);
 p.pads[0].gain=400;assert(studio_pressure(3000,p.pads[0])==4095);
 p.pads[0].enabled=false;assert(studio_pressure(3000,p.pads[0])==0);
 char path[]="/tmp/mk1-profile-test-XXXXXX";int fd=mkstemp(path);assert(fd>=0);
 const char *invalid="<?xml version=\"1.0\"?><plist version=\"1.0\"><dict><key>pads</key><string>bad</string></dict></plist>";
 assert(write(fd,invalid,strlen(invalid))==(ssize_t)strlen(invalid));close(fd);
 snprintf(g_studio_profile_path,sizeof(g_studio_profile_path),"%s",path);
 assert(!studio_read_profile(&p));unlink(path);assert(studio_read_profile(&p));
 puts("PASS: physical profile defaults, gain clamp, disable and malformed plist rejection");
}

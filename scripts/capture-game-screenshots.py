#!/usr/bin/env python3
"""Capture real game rendering without photographing the physical LCDs."""
from pathlib import Path
import subprocess, tempfile, os
from PIL import Image, ImageDraw, ImageFont
root=Path(__file__).resolve().parents[1]
out=root/'docs/screenshots';out.mkdir(exist_ok=True)
s=(root/'mk1-bridge/main.c').read_text()
renderer=s[s.index('#define STATUS_SCALE'):s.index('#include "freedoom_client.h"')]
pre='''#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mk1-bridge/idle_games.h"
#define DISPLAY_BYTES_PER_ROW 170
'''
main='''
static void save(const char *path,uint8_t *fb){
 FILE *f=fopen(path,"wb");fprintf(f,"P5\\n255 64\\n255\\n");
 for(int y=0;y<64;y++)for(int g=0;g<85;g++){
  int a=fb[y*170+g*2],b=fb[y*170+g*2+1];
  fputc((a>>3)*255/31,f);fputc((((a&7)<<2)|(b>>6))*255/31,f);fputc((b&31)*255/31,f);
 }fclose(f);
}
int main(){uint8_t fb[10880];idle_pong_t p;idle_pong_reset(&p);
 for(int i=0;i<110;i++)idle_pong_step(&p,.02f);
 render_pong_fb(fb,&p);save("pong.pgm",fb);
 idle_breakout_t b;idle_breakout_reset(&b);idle_breakout_launch(&b);
 for(int i=0;i<45;i++)idle_breakout_step(&b,.02f,0);
 render_breakout_fb(fb,&b);save("breakout.pgm",fb);
 render_clock_fb(fb,time(NULL));save("clock.pgm",fb);
}
'''
with tempfile.TemporaryDirectory(prefix='mk1-screenshots-') as tmp:
 tmp=Path(tmp);(tmp/'capture.c').write_text(pre+renderer+main)
 subprocess.run(['clang','-I'+str(root),str(tmp/'capture.c'),'-lm','-o',str(tmp/'capture')],check=True)
 subprocess.run([str(tmp/'capture')],cwd=tmp,env=dict(os.environ,TZ='Europe/Tallinn'),check=True)
 subprocess.run(['python3','tests/test_freedoom_runtime.py'],cwd=root,check=True)
 def panel(name,left,right):
  image=Image.new('RGB',(1584,310),'#111616');draw=ImageDraw.Draw(image)
  font=ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial.ttf',20)
  draw.text((24,14),name+' — display framebuffer capture',fill='#63e5ce',font=font)
  for x,path,title in [(24,left,'LEFT LCD — GAME'),(810,right,'RIGHT LCD — '+('ORIGINAL STATUS BAR' if name=='FREEDOOM' else 'CLOCK / DATE'))]:
   pixels=Image.open(path).convert('RGB').resize((765,192),Image.Resampling.NEAREST)
   image.paste(pixels,(x,60));draw.text((x,270),title,fill='#cad5d0',font=font)
  image.save(out/(name.lower()+'.png'))
 panel('PONG',tmp/'pong.pgm',tmp/'clock.pgm')
 panel('BREAKOUT',tmp/'breakout.pgm',tmp/'clock.pgm')
 panel('FREEDOOM',Path('/tmp/mk1-real-freedoom.pgm'),Path('/tmp/mk1-native-hud.pgm'))

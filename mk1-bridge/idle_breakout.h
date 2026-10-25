#ifndef MK1_IDLE_BREAKOUT_H
#define MK1_IDLE_BREAKOUT_H
#include "idle_pong.h"
typedef struct {
 float x,y,vx,vy,paddle;
 uint32_t bricks;
 unsigned lives,score;
 bool waiting,won;
} idle_breakout_t;
static inline void idle_breakout_reset(idle_breakout_t *b) {
 *b=(idle_breakout_t){.x=127,.y=49,.vx=45,.vy=-38,.paddle=127,
                    .bricks=0xffffffffu,.lives=3,.waiting=true};
}
static inline void idle_breakout_launch(idle_breakout_t *b) {
 if(!b->lives||b->won)idle_breakout_reset(b);
 b->waiting=false;
}
static inline void idle_breakout_encoder(idle_breakout_t *b,float delta) {
 if(isfinite(delta))b->paddle=idle_pong_clamp(b->paddle+delta*180,24,230);
}
static inline void idle_breakout_step(idle_breakout_t *b,float dt,uint32_t pads) {
 if(!isfinite(dt)||dt<=0||!b->lives||b->won)return;
 dt=fminf(dt,.05f);
 int move=(int)((pads>>3)&1)-(int)(pads&1);
 b->paddle=idle_pong_clamp(b->paddle+move*160*dt,24,230);
 if(b->waiting){b->x=b->paddle;b->y=53;return;}
 float prev_y=b->y;b->x+=b->vx*dt;b->y+=b->vy*dt;
 if(b->x<3){b->x=6-b->x;b->vx=fabsf(b->vx);}
 if(b->x>251){b->x=502-b->x;b->vx=-fabsf(b->vx);}
 if(b->y<10){b->y=20-b->y;b->vy=fabsf(b->vy);}
 if(b->vy>0&&prev_y<=55&&b->y>=55&&fabsf(b->x-b->paddle)<=25){
  b->y=110-b->y;b->vy=-fabsf(b->vy)-1;b->vx=(b->x-b->paddle)*3;
 }
 for(unsigned i=0;i<32;i++)if(b->bricks&(1u<<i)){
  int x=5+(i%8)*31,y=13+(i/8)*6;
  if(b->x>=x-2&&b->x<=x+28&&b->y>=y-2&&b->y<=y+4){
   b->bricks&=~(1u<<i);b->score+=10;b->vy=-b->vy;break;
  }
 }
 if(!b->bricks){b->won=true;return;}
 if(b->y>65){b->lives--;b->waiting=true;b->vx=45;b->vy=-38;}
}
#endif

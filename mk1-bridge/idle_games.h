#ifndef MK1_IDLE_GAMES_H
#define MK1_IDLE_GAMES_H
#include "idle_pong.h"
#include "idle_breakout.h"

typedef struct {
 idle_pong_t pong;
 idle_breakout_t breakout;
 bool menu;
 int selected; /* 0=Pong, 1=Freedoom, 2=Breakout */
 float selection_delta;
} idle_games_t;
static inline void idle_games_reset(idle_games_t *g) {
 *g=(idle_games_t){.menu=true};
 idle_pong_reset(&g->pong);idle_breakout_reset(&g->breakout);
}
static inline void idle_games_start(idle_games_t *g) {
 if(g->selected==0)idle_pong_reset(&g->pong);
 if(g->selected==2){idle_breakout_reset(&g->breakout);idle_breakout_launch(&g->breakout);}
 g->menu=false;g->selection_delta=0;
}
static inline bool idle_games_encoder(idle_games_t *g,uint32_t index,float delta) {
 if(!isfinite(delta)||delta==0)return false;
 if(index==5){ /* Third encoder below left display, physical order 1..8. */
  g->menu=true;g->selection_delta+=delta;
  if(g->selection_delta>.025f){g->selected=(g->selected+1)%3;g->selection_delta=0;}
  else if(g->selection_delta<-.025f){g->selected=(g->selected+2)%3;g->selection_delta=0;}
  return true;
 }
 if(g->menu)return false;
 if(g->selected==2&&index==3){idle_breakout_encoder(&g->breakout,delta);return true;}
 return g->selected==0?idle_pong_encoder(&g->pong,index,delta):false;
}
static inline bool idle_games_button(idle_games_t *g,uint32_t index) {
 if(index==0x1a||index==0x1d){
  g->menu=true;g->selected=(g->selected+(index==0x1d?1:2))%3;g->selection_delta=0;return true;
 }
 if(index==0x19){g->menu=true;g->selection_delta=0;return true;} /* Browse */
 if(index==0x29){ /* Play */
  if(g->menu)idle_games_start(g);
  else if(g->selected==2)idle_breakout_launch(&g->breakout);
  return true;
 }
 if(index==0x0f){idle_games_start(g);return true;} /* Restart */
 return false;
}
static inline void idle_games_step(idle_games_t *g,float dt,uint32_t pads) {
 if(g->menu)return;
 if(g->selected==0)idle_pong_step(&g->pong,dt);
 if(g->selected==2)idle_breakout_step(&g->breakout,dt,pads);
}
#endif

#include "../mk1-bridge/idle_games.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 idle_games_t g;idle_games_reset(&g);
 idle_games_button(&g,0x1d);assert(g.menu&&g.selected==1);
 idle_games_button(&g,0x1d);assert(g.selected==2);
 idle_games_button(&g,0x29);assert(!g.menu&&!g.breakout.waiting);
 idle_games_button(&g,0x1a);assert(g.menu&&g.selected==1);
 idle_games_button(&g,0x1a);assert(g.selected==0);
 idle_games_button(&g,0x1a);assert(g.selected==2);
 idle_pong_t p;idle_pong_reset(&p);idle_pong_encoder(&p,6,.1f);assert(p.right_manual);
 idle_pong_step(&p,9.9f);assert(p.left_idle<10&&p.right_manual);
 idle_pong_step(&p,.11f);assert(p.left_idle>=10&&!p.right_manual);
 idle_pong_encoder(&p,3,.1f);assert(p.left_idle==0);
 idle_pong_encoder(&p,6,.1f);assert(p.right_manual&&p.right_idle==0);
 idle_breakout_t b;idle_breakout_reset(&b);idle_breakout_launch(&b);
 b.x=10;b.y=12;b.vx=0;b.vy=30;idle_breakout_step(&b,.05f,0);assert(b.score==10);
 b.y=66;b.vy=30;idle_breakout_step(&b,.05f,0);assert(b.lives==2&&b.waiting);
 puts("PASS: hardware arrow selection, 10-second AI handoff/takeover, Breakout scoring/lives");
}

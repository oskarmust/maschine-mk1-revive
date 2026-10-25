#include "../mk1-bridge/idle_games.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 idle_games_t g;idle_games_reset(&g);assert(g.menu&&g.selected==0);
 assert(!idle_games_encoder(&g,5,NAN));
 idle_games_encoder(&g,5,.03f);assert(g.menu&&g.selected==1);
 idle_games_button(&g,0x29);assert(!g.menu);
 idle_games_encoder(&g,5,-.03f);assert(g.menu&&g.selected==0);
 idle_games_button(&g,0x29);assert(!g.menu&&!g.pong.right_manual);
 idle_games_encoder(&g,6,.1f);assert(g.pong.right_manual);
 idle_games_button(&g,0x19);assert(g.menu);
 idle_games_button(&g,0x0f);assert(!g.menu&&!g.pong.right_manual);
 puts("PASS: third encoder selects games; Play, Browse, Restart and Pong handoff");
}

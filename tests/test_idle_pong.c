#include "../mk1-bridge/idle_pong.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    idle_pong_t p;
    idle_pong_reset(&p);
    assert(p.left_score == 0 && !p.right_manual);
    assert(idle_pong_encoder(&p, 3, 100) && p.left_y == 54);
    assert(idle_pong_encoder(&p, 3, -100) && p.left_y == 23);
    assert(!idle_pong_encoder(&p, 0, 1) && !p.right_manual);
    assert(!idle_pong_encoder(&p, 6, NAN) && !p.right_manual);
    assert(idle_pong_encoder(&p, 6, .1f) && p.right_manual);
    float right = p.right_y;
    idle_pong_step(&p, .05f);
    assert(p.right_y == right); // Human control must disable the AI.
    p.serve_wait=0; p.x=14; p.y=p.left_y; p.vx=-82; p.vy=0;
    idle_pong_step(&p,.05f); assert(p.vx>0 && p.right_score==0);
    p.x=240; p.y=p.right_y; p.vx=82;
    idle_pong_step(&p,.05f); assert(p.vx<0 && p.left_score==0);
    p.x=0; p.y=59; p.vx=-100; p.vy=0;
    idle_pong_step(&p,.05f); assert(p.right_score==1 && p.serve_wait>0);
    p.serve_wait=0; p.x=256; p.vx=100;
    idle_pong_step(&p,.05f); assert(p.left_score==1);
    idle_pong_reset(&p);
    for (int i=0; i<20000; i++) {
        idle_pong_step(&p,.05f);
        assert(isfinite(p.x) && isfinite(p.y) && p.y>=17 && p.y<=60);
        assert(p.right_y>=23 && p.right_y<=54);
    }
    puts("PASS: encoders, AI handoff, paddle collisions, scoring, long-run bounds");
}

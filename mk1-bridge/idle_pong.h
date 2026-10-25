#ifndef MK1_IDLE_PONG_H
#define MK1_IDLE_PONG_H
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/* Logical display coordinates: 255 x 64, playfield below the score. */
typedef struct {
    float x, y, vx, vy, left_y, right_y, serve_wait;
    unsigned left_score, right_score;
    bool right_manual;
    float left_idle, right_idle;
} idle_pong_t;

static inline float idle_pong_clamp(float v, float lo, float hi)
{ return v < lo ? lo : (v > hi ? hi : v); }

static inline void idle_pong_serve(idle_pong_t *p, bool to_right)
{
    p->x = 127; p->y = 38;
    p->vx = to_right ? 82 : -82;
    p->vy = ((p->left_score + p->right_score) & 1) ? -25 : 25;
    p->serve_wait = 0.7f;
}

static inline void idle_pong_reset(idle_pong_t *p)
{
    *p = (idle_pong_t){ .left_y = 38, .right_y = 38, .right_idle = 10 };
    idle_pong_serve(p, true);
}

/* Only the outside encoders below the left display control the game. */
static inline bool idle_pong_encoder(idle_pong_t *p, uint32_t index, float delta)
{
    if (!isfinite(delta) || delta == 0) return false;
    if (index == 3) {
        p->left_idle = 0;
        p->left_y = idle_pong_clamp(p->left_y + delta * 96, 23, 54);
        return true;
    }
    if (index == 6) {
        p->right_manual = true;
        p->right_idle = 0;
        p->right_y = idle_pong_clamp(p->right_y + delta * 96, 23, 54);
        return true;
    }
    return false;
}

static inline void idle_pong_step(idle_pong_t *p, float dt)
{
    if (!isfinite(dt) || dt <= 0) return;
    p->left_idle = fminf(10,p->left_idle+dt);
    p->right_idle = fminf(10,p->right_idle+dt);
    if(p->right_idle>=10)p->right_manual=false;
    dt = fminf(dt, 0.05f); /* No giant jump after USB delays or system sleep. */
    if(p->left_idle>=10){
        float target=p->vx<0?p->y:38;
        p->left_y=idle_pong_clamp(p->left_y+
            idle_pong_clamp(target-p->left_y,-22*dt,22*dt),23,54);
    }
    if (!p->right_manual) {
        float target = p->vx > 0 ? p->y : 38;
        p->right_y = idle_pong_clamp(p->right_y +
            idle_pong_clamp(target - p->right_y, -22 * dt, 22 * dt), 23, 54);
    }
    if (p->serve_wait > 0) {
        p->serve_wait = fmaxf(0, p->serve_wait - dt);
        return;
    }
    float prev_x = p->x;
    p->x += p->vx * dt; p->y += p->vy * dt;
    if (p->y < 17) { p->y = 34 - p->y; p->vy = fabsf(p->vy); }
    if (p->y > 60) { p->y = 120 - p->y; p->vy = -fabsf(p->vy); }
    if (p->vx < 0 && prev_x >= 13 && p->x <= 13 && fabsf(p->y - p->left_y) <= 10) {
        p->x = 26 - p->x;
        p->vx = fminf(fabsf(p->vx) + 4, 135);
        float bounce = (p->y - p->left_y) * 6;
        p->vy = fabsf(bounce) < 15 ? copysignf(15, p->vy == 0 ? 1 : p->vy) : bounce;
    }
    if (p->vx > 0 && prev_x <= 241 && p->x >= 241 && fabsf(p->y - p->right_y) <= 10) {
        p->x = 482 - p->x;
        p->vx = -fminf(fabsf(p->vx) + 4, 135);
        float bounce = (p->y - p->right_y) * 6;
        p->vy = fabsf(bounce) < 15 ? copysignf(15, p->vy == 0 ? 1 : p->vy) : bounce;
    }
    if (p->x < -2) {
        p->right_score = (p->right_score + 1) % 100;
        idle_pong_serve(p, false);
    } else if (p->x > 257) {
        p->left_score = (p->left_score + 1) % 100;
        idle_pong_serve(p, true);
    }
}
#endif

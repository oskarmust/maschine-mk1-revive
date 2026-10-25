#ifndef MK1_FREEDOOM_PROTOCOL_H
#define MK1_FREEDOOM_PROTOCOL_H
#include <stdint.h>
#include <stdatomic.h>
#define MK1_DOOM_W 255
#define MK1_DOOM_H 64
#define MK1_DOOM_MAGIC 0x4d4b3145u
typedef struct {
 uint32_t magic;
 uint32_t owner_pid;
 _Atomic uint32_t sequence;
 _Atomic uint32_t keys;
 _Atomic int32_t player_x, player_y;
 _Atomic int32_t health, armor, ammo, kills, total_kills, episode, map, cards;
 uint8_t pixels[MK1_DOOM_W*MK1_DOOM_H];
 uint8_t hud_pixels[MK1_DOOM_W*MK1_DOOM_H];
} mk1_doom_shared_t;
/* Pad bits 0..15 plus short pulses from rotary encoders / Play. */
#define MK1_DOOM_TURN_LEFT (1u<<16)
#define MK1_DOOM_TURN_RIGHT (1u<<17)
#define MK1_DOOM_FORWARD (1u<<18)
#define MK1_DOOM_BACKWARD (1u<<19)
#define MK1_DOOM_FIRE (1u<<20)
#endif

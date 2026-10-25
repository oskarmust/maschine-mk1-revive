#ifndef MK1_IDLE_GAME_LEDS_H
#define MK1_IDLE_GAME_LEDS_H
#include "idle_games.h"
#include <string.h>
/* Packet A: rubber pads are in reversed columns, bottom row first. */
static inline void idle_games_led_packet(const idle_games_t *g,uint32_t held,uint8_t packet[33])
{
    memset(packet,0,33);packet[0]=0x0c;packet[1]=0x00;
    if(g->menu){packet[2+29]=0x32;return;} /* Play: start selected game. */
    if(g->selected==2){packet[2+3]=40;packet[2+0]=40;packet[2+29]=32;return;}
    if(g->selected!=1)return; /* Pong uses encoders, not pads. */
    static const uint8_t levels[16]={
        0,0,0,0, 0,0,0,0,
        40,40,40,63, /* 9=A, 10=S, 11=D, 12=fire */
        12,40,12,28  /* 13=strafe left, 14=W, 15=strafe right, 16=use */
    };
    for(unsigned index=0;index<16;index++){
        uint8_t level=levels[index];
        if(level && (held&(1u<<index)))level=63;
        unsigned slot=(index/4)*4+(3-index%4);
        packet[2+slot]=level;
    }
}
#endif

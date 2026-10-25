#include "../mk1-bridge/idle_game_leds.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 idle_games_t g;idle_games_reset(&g);uint8_t packet[33];
 idle_games_led_packet(&g,0,packet);assert(packet[0]==0x0c&&packet[1]==0);
 for(int i=0;i<16;i++)assert(packet[2+i]==0);
 assert(packet[31]==50); /* Menu Play indicator. */
 g.menu=false;g.selected=1;idle_games_led_packet(&g,0,packet);
 /* Physical LED order: 12,11,10,9 / 16,15,14,13, not pad-number order. */
 assert(packet[2+8]==63&&packet[2+9]==40&&packet[2+10]==40&&packet[2+11]==40);
 assert(packet[2+12]==28&&packet[2+13]==12&&packet[2+14]==40&&packet[2+15]==12);
 for(int i=0;i<8;i++)assert(packet[2+i]==0);
 for(int i=16;i<31;i++)assert(packet[2+i]==0); /* No unrelated buttons/backlight. */
 idle_games_led_packet(&g,1u<<13,packet);assert(packet[2+14]==63);
 idle_games_led_packet(&g,0,packet);assert(packet[2+14]==40);
 g.menu=true;idle_games_led_packet(&g,0,packet);
 for(int i=0;i<16;i++)assert(packet[2+i]==0);
 g.menu=false;g.selected=0;idle_games_led_packet(&g,0,packet);
 for(int i=2;i<33;i++)assert(packet[i]==0);
 puts("PASS: WASD/action physical LED order, brightness, press/release, menu/Pong clear");
}

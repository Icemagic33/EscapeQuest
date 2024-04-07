#ifndef GLOBAL
#define GLOBAL

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define COLOR_BACKGROUND 0x0000 /* Black, or transparent */
#define COLOR_HAIR 0x7C1F       /* Brown hair */
#define COLOR_SKIN 0xFEA0       /* Skin tone */
#define COLOR_SHIRT 0x03BF      /* Blue shirt */
#define COLOR_SHIRT_RED 0xF800
#define COLOR_PANTS 0x001F /* Dark blue pants */

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CHARACTER_WIDTH 15
#define CHARACTER_HEIGHT 15
#define PS2_BASE 0xFF200100
#define TIMER_BASE ((volatile int *)0xFF202000)
#define HEX_BASE ((volatile int *)0xFF200020)  // HEX display

#define BUFFER_WIDTH 512         // Buffer width
#define BUFFER_HEIGHT 240        // Height matches the display height
#define COUNTER_DELAY 100000000  // 1-second interval at 100 MHz clock
#define RED 0xF800

struct Maze {
  int x;
  int y;
  int color;
};

extern const uint16_t maze_pacman[240][320];
extern const uint16_t menu[240][320];

#endif
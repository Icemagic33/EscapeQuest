#ifndef GLOBAL
#define GLOBAL

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Include the header for memcpy function
#include <time.h>    // Inlcude for random destination generation

#define COLOR_BACKGROUND 0x0000 /* Black, or transparent */
#define COLOR_HAIR 0x7C1F       /* Brown hair */
#define COLOR_SKIN 0xFEA0       /* Skin tone */
#define COLOR_SHIRT 0x03BF      /* Blue shirt */
#define COLOR_SHIRT_RED 0xF800
#define COLOR_PANTS 0x001F  /* Dark blue pants */
#define COLOR_GREEN 0x07E0  // Assuming 5-6-5 bit RGB for green

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CHARACTER_WIDTH 11
#define CHARACTER_HEIGHT 11
#define DESTINATION_SIZE 5
#define PS2_BASE 0xFF200100
#define TIMER_BASE ((volatile int *)0xFF202000)
#define HEX_BASE ((volatile int *)0xFF200020)  // HEX display

#define BUFFER_WIDTH 512         // Buffer width
#define BUFFER_HEIGHT 240        // Height matches the display height
#define COUNTER_DELAY 100000000  // 1-second interval at 100 MHz clock
#define RED 0xF800

typedef struct {
  int x;  // Top-left x coordinate of the destination
  int y;  // Top-left y coordinate of the destination
} Destination;

volatile int pixel_buffer_start;  // global variable for the pixel buffer
short int Buffer1[240][512];      // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
uint32_t last_displayed_number =
    -1;  // Initialize to an invalid number to ensure update on first loop

extern const uint16_t menu[240][320];
extern const uint16_t maze2[240][320];
extern const uint16_t maze3[240][320];

// Function prototypes
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void wait_for_vsync();
void check_and_correct_boundaries(int *x, int *y);
void setup_timer();
void move_player1_2(uint8_t scan_code, int *x, int *y);
void move_player2_2(uint8_t scan_code, int *x, int *y);
void move_player1_3(uint8_t scan_code, int *x, int *y);
void move_player2_3(uint8_t scan_code, int *x, int *y);
void draw_character(int top_left_x, int top_left_y, unsigned int color,
                    int num);
void display_hex(uint32_t number);
bool is_blocked2(int x, int y);
bool is_blocked3(int x, int y);
bool has_caught(int p1_x, int p1_y, int p2_x, int p2_y);
// Destination create_dynamic_destination(int maze[320][240]);
Destination create_dynamic_destination_2();
Destination create_dynamic_destination_3();
void draw_destination(Destination dest);
void clear_destination(Destination dest);
bool has_reached_destination(int p1_x, int p1_y, Destination dest);
void clear_hex_display(void);
void audio_play_for_win(int *samples, int sample_n_1);
void draw_screen(const uint16_t draw[240][320]);
void audio_play_for_lose(int *samples, int sample_n_1);

int value;
// Digit to 7-segment encoding for common anode HEX display
int digit_to_segment[10] = {
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x6F,  // 9
};
#endif
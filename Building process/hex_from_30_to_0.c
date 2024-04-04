#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define COLOR_BACKGROUND 0x0000 /* Black, or transparent */
#define COLOR_HAIR 0x7C1F       /* Brown hair */
#define COLOR_SKIN 0xFEA0       /* Skin tone */
#define COLOR_SHIRT 0x03BF      /* Blue shirt */
#define COLOR_PANTS 0x001F      /* Dark blue pants */

unsigned int character[15][15] = {
    // Top rows (mostly hair)
    {COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_HAIR,
     COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR,
     COLOR_HAIR, COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND,
     COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR,
     COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR,
     COLOR_HAIR, COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_HAIR, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN,
     COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_HAIR, COLOR_SKIN, COLOR_SKIN,
     COLOR_SKIN, COLOR_HAIR, COLOR_BACKGROUND, COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_HAIR, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN,
     COLOR_SKIN, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN,
     COLOR_SKIN, COLOR_HAIR, COLOR_BACKGROUND, COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN,
     COLOR_SKIN, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN, COLOR_SKIN,
     COLOR_SKIN, COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND},

    // Shirt and arms
    {COLOR_BACKGROUND, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SKIN,
     COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT,
     COLOR_SKIN, COLOR_SHIRT, COLOR_SHIRT, COLOR_BACKGROUND, COLOR_BACKGROUND},
    {COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT,
     COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT,
     COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_BACKGROUND},
    {COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT,
     COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT,
     COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT},
    {COLOR_BACKGROUND, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT,
     COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT,
     COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_SHIRT, COLOR_BACKGROUND},

    // Pants and legs
    {COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS,
     COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS,
     COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS,
     COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS,
     COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS, COLOR_BACKGROUND,
     COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS,
     COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS,
     COLOR_BACKGROUND, COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS, COLOR_BACKGROUND,
     COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS, COLOR_PANTS,
     COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_PANTS, COLOR_PANTS,
     COLOR_BACKGROUND, COLOR_BACKGROUND},
    {COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND,
     COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND,
     COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND,
     COLOR_BACKGROUND, COLOR_BACKGROUND, COLOR_BACKGROUND}};

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define PS2_BASE 0xFF200100
#define TIMER_BASE ((volatile int *)0xFF202000)
#define HEX_BASE ((volatile int *)0xFF200020)  // HEX display

#define BUFFER_WIDTH 512         // Buffer width
#define BUFFER_HEIGHT 240        // Height matches the display height
#define COUNTER_DELAY 100000000  // 1-second interval at 100 MHz clock
#define RED 0xF800

volatile int pixel_buffer_start;  // global variable
short int Buffer1[240][512];      // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
uint32_t last_displayed_number =
    -1;  // Initialize to an invalid number to ensure update on first loop

// Function prototypes
void clear_screen();
void plot_pixel(int x, int y, short int line_color);
void wait_for_vsync();
void check_and_correct_boundaries(int *x, int *y);
void setup_timer();
void delay_1sec();
void fill_screen_with_color(short int color);
void move_player(uint8_t scan_code, int *x, int *y);
void draw_player1(int x, int y);
void draw_character(int top_left_x, int top_left_y, unsigned int color);
void display_hex(uint32_t number);

short int Buffer1[BUFFER_HEIGHT][BUFFER_WIDTH];  // Declared buffer
volatile int pixel_buffer_start;  // Global variable for the pixel buffer

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
    0x6F   // 9
};

int main(void) {
  volatile int *PS2_ptr = (int *)PS2_BASE;  // PS/2 base address
  volatile int *pixel_ctrl_ptr =
      (int *)0xFF203020;        // Pixel controller base address
  int buffer = 0;               // Start with Buffer1
  unsigned long last_time = 0;  // Last time we updated the countdown

  setup_timer();          // Initialize the timer for countdown
  uint32_t counter = 30;  // Start counting from 9

  int PS2_data, RVALID;
  char byte3 = 0;

  int x = SCREEN_WIDTH / 2;  // Dot initial position (center)
  int y = SCREEN_HEIGHT / 2;

  /* set front pixel buffer to Buffer 1 */
  *(pixel_ctrl_ptr + 1) =
      (int)&Buffer1;  // first store the address in the  back buffer
  /* now, swap the front/back buffers, to set the front buffer location */
  wait_for_vsync();
  /* initialize a pointer to the pixel buffer, used by drawing functions */
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();  // pixel_buffer_start points to the pixel buffer

  /* set back pixel buffer to Buffer 2 */
  *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // we draw on the back buffer
  clear_screen();  // pixel_buffer_start points to the pixel buffer

  // PS/2 keyboard reset
  *PS2_ptr = 0xFF;  // Reset

  // Assuming the initial position of the box is in the center
  int x_prev = x, y_prev = y;  // Position from the last frame
  int x_old = x, y_old = y;    // Position from two frames ago

  while (1) {
    display_hex(counter);
    // unsigned long current_time = *TIMER_BASE;  // Assume TIMER_BASE points to
    // a readable timer

    // Check for PS/2 keyboard inputs
    PS2_data = *PS2_ptr;         // Read the Data register in the PS/2 port
    RVALID = PS2_data & 0x8000;  // Extract the RVALID field
    // Inside the main loop, before clear_screen()
    if (RVALID) {
      // Move player based on the current input
      byte3 = PS2_data & 0xFF;
      move_player(byte3, &x, &y);  // This updates the current position

      // Erase the box from two frames ago
      draw_character(
          x_old, y_old,
          COLOR_BACKGROUND);  // Erases by drawing with background color

      // Draw the box in its new position
      draw_character(x, y,
                     0xFFFF);  // draws the character with its appropriate color

      // Update positions for the next frame
      x_old = x_prev;
      y_old = y_prev;
      x_prev = x;
      y_prev = y;

      wait_for_vsync();  // Swap buffers for smooth animation
      *(pixel_ctrl_ptr + 1) = pixel_buffer_start;  // Update back buffer address
    }

    if (TIMER_BASE[0] & 0x1) {
      TIMER_BASE[0] = 0;  // Reset TO bit to count another second
      if (counter > 0) {
        counter--;
      } else {
        break;  // Exit loop when counter reaches 0
      }
    }
  }

  fill_screen_with_color(RED);  // Fill the screen with red
  wait_for_vsync();

  return 0;
}

void fill_screen_with_color(short int color) {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      plot_pixel(x, y, color);  // Fill each pixel with the given color
    }
  }
}

void setup_timer() {
  // Clear TO bit and set delay for 1-second intervals
  TIMER_BASE[0] = 0;                       // Clear TO bit
  TIMER_BASE[2] = COUNTER_DELAY & 0xFFFF;  // Set period low
  TIMER_BASE[3] = COUNTER_DELAY >> 16;     // Set period high
  TIMER_BASE[1] = 0b0110;                  // Timer mode: continuous and start
}

void delay_1sec() {
  // Reset the timer to count from 0 again for 1 second
  TIMER_BASE[0] = 0;  // Clear TO bit to start counting from 0
  while (!(TIMER_BASE[0] & 0b1))
    ;                 // Wait if TO bit is not set
  TIMER_BASE[0] = 0;  // Clear TO bit for the next interval
}

void display_red_screen() {
  clear_screen();  // Optionally clear the screen first if not already clear
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      plot_pixel(x, y, RED);  // Fill the screen with red
    }
  }
  wait_for_vsync();  // Ensure red screen is displayed
}

void plot_pixel(int x, int y, short int line_color) {
  *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      plot_pixel(x, y, 0x0000);  // all black
    }
  }
}

void wait_for_vsync() {
  volatile int *pixel_ctrl_ptr =
      (int *)0xFF203020;  // Pixel controller base address
  register int status;

  *pixel_ctrl_ptr = 1;  // Start the synchronization process
  status = *(pixel_ctrl_ptr + 3);
  while ((status & 0x01) != 0) {
    status = *(pixel_ctrl_ptr + 3);
  }
}

void check_and_correct_boundaries(int *x, int *y) {
  if (*x < 0) {
    *x = 0;
  } else if (*x + 15 >= SCREEN_WIDTH) {
    *x = SCREEN_WIDTH - 1;
  }

  if (*y < 0) {
    *y = 0;
  } else if (*y + 15 >= SCREEN_HEIGHT) {
    *y = SCREEN_HEIGHT - 1;
  }
}

void move_player(uint8_t scan_code, int *x, int *y) {
  if (scan_code == 0x1C) {  // 'A' pressed, move left
    *x = *x > 0 ? *x - 1 : *x;
  } else if (scan_code == 0x1D) {  // 'W' pressed, move up
    *y = *y > 0 ? *y - 1 : *y;
  } else if (scan_code == 0x1B) {  // 'S' pressed, move down
    *y = *y < SCREEN_HEIGHT - 1 ? *y + 1 : *y;
  } else if (scan_code == 0x23) {  // 'D' pressed, move right
    *x = *x < SCREEN_WIDTH - 1 ? *x + 1 : *x;
  }

  check_and_correct_boundaries(x, y);
}

// void draw_character(int top_left_x, int top_left_y, int color) {
//   for (int y = 0; y < 15; ++y) {
//     for (int x = 0; x < 15; ++x) {
//       plot_pixel(top_left_x + x, top_left_y + y, character[y][x]);
//     }
//   }
// }

void draw_character(int top_left_x, int top_left_y, unsigned int color) {
  // If the color is background, we're erasing the character
  bool erase = (color == COLOR_BACKGROUND);

  for (int y = 0; y < 15; ++y) {
    for (int x = 0; x < 15; ++x) {
      // Draw or erase depending on the color
      unsigned int draw_color = erase ? COLOR_BACKGROUND : character[y][x];
      plot_pixel(top_left_x + x, top_left_y + y, draw_color);
    }
  }
}

void display_hex(uint32_t number) {
  if (last_displayed_number != number) {
    uint32_t tens = number / 10;  // Get the tens digit
    uint32_t ones = number % 10;  // Get the ones digit

    // Combine the segments for HEX1 and HEX0
    uint32_t combined_segments =
        (digit_to_segment[tens] << 8) | digit_to_segment[ones];

    // Update the HEX display only if the number has changed
    *HEX_BASE = combined_segments;

    // Remember the last displayed number
    last_displayed_number = number;
  }
}

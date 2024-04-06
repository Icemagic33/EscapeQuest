#include "characters.c"
#include "globals.h"
#include "menu.c"
#include "pacman_maze.c"

volatile int pixel_buffer_start;  // global variable for the pixel buffer
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
void move_player1(uint8_t scan_code, int *x, int *y);
void move_player2(uint8_t scan_code, int *x, int *y);
void draw_player1(int x, int y);
void draw_character(int top_left_x, int top_left_y, unsigned int color,
                    int num);
void display_hex(uint32_t number);
bool is_blocked(int x, int y);

// global variable
// int pixel_buffer_start;
// short int Buffer1[240][512];
// short int Buffer2[240][512];
int value;
// void plot_pixel(int x, int y, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int color);
// void clear_screen();
void swap(int *x, int *y);
void draw_frame();
void draw_entrance_exit();
void draw_maze_level1();
void draw_maze_level2();
void draw_menu();
// void wait_for_vsync();

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
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  volatile int *SW_ptr = (int *)0xff200040;
  volatile int *LEDR_ptr = (int *)0xff200000;

  /* Read location of the pixel buffer from the pixel buffer controller */
  pixel_buffer_start = *pixel_ctrl_ptr;

  *(pixel_ctrl_ptr + 1) = (int)Buffer1;  // Set front buffer
  wait_for_vsync();
  pixel_buffer_start =
      *pixel_ctrl_ptr;  // Use the current front buffer to clear screen
  draw_menu();
  *(pixel_ctrl_ptr + 1) = (int)Buffer2;        // Set back buffer
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // Draw on back buffer

  while (1) {
    int value = *SW_ptr;
    if (value == 1) {
      *LEDR_ptr = value;
      draw_frame();
      draw_entrance_exit();
      draw_maze_level1();
      wait_for_vsync();
      pixel_buffer_start = *(pixel_ctrl_ptr + 1);
      break;
    }
    if (value == 2) {
      *LEDR_ptr = value;
      draw_frame();
      draw_entrance_exit();
      draw_maze_level2();
      wait_for_vsync();
      pixel_buffer_start = *(pixel_ctrl_ptr + 1);
      break;
    }
  }
  // for level select
  // draw_frame() ;
  // draw_entrance_exit() ;
  volatile int *PS2_ptr = (int *)PS2_BASE;  // PS/2 base address
  //   volatile int *pixel_ctrl_ptr = (int *)0xFF203020;        // Pixel
  //   controller base address
  int buffer = 0;               // Start with Buffer1
  unsigned long last_time = 0;  // Last time we updated the countdown

  setup_timer();          // Initialize the timer for countdown
  uint32_t counter = 30;  // Start counting from 9

  int PS2_data, RVALID;
  char byte3 = 0;

  int x1 = 17;  // Player 1 entrance
  int y1 = 205;
  int x2 = 280;  // Player 2 entrance
  int y2 = 205;

  /* set front pixel buffer to Buffer 1 */
  *(pixel_ctrl_ptr + 1) =
      (int)&Buffer1;  // first store the address in the  back buffer
  /* now, swap the front/back buffers, to set the front buffer location */
  wait_for_vsync();
  /* initialize a pointer to the pixel buffer, used by drawing functions */
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();  // pixel_buffer_start points to the pixel buffer
                   //   draw_frame();
                   //   draw_entrance_exit();
                   //   draw_maze_level2();
  draw_maze();
  wait_for_vsync();

  /* set back pixel buffer to Buffer 2 */
  *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // we draw on the back buffer
  clear_screen();  // pixel_buffer_start points to the pixel buffer
                   //   draw_frame();
                   //   draw_entrance_exit();
                   //   draw_maze_level2();
  draw_maze();
  wait_for_vsync();

  // PS/2 keyboard reset
  *PS2_ptr = 0xFF;  // Reset

  // Assuming the initial position of the box is in the center
  int x1_prev = x1, y1_prev = y1;  // Position from the last frame
  int x1_old = x1, y1_old = y1;    // Position from two frames ago
  int x2_prev = x2, y2_prev = y2;  // Position from the last frame
  int x2_old = x2, y2_old = y2;    // Position from two frames ago
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
      move_player1(byte3, &x1, &y1);  // This updates the current position

      move_player2(byte3, &x2, &y2);  // This updates the current position

      // Erase the box from two frames ago
      draw_character(x1_old, y1_old, COLOR_BACKGROUND,
                     0);  // Erases by drawing with background color
      draw_character(x2_old, y2_old, COLOR_BACKGROUND,
                     1);  // Erases by drawing with background color

      // Draw the character in its new position
      draw_character(x1, y1, 0xFFFF,
                     0);  // draws the character with its appropriate color

      draw_character(x2, y2, 0xFFFF, 1);
      // Update positions for the next frame
      x1_old = x1_prev;
      y1_old = y1_prev;
      x1_prev = x1;
      y1_prev = y1;

      x2_old = x2_prev;
      y2_old = y2_prev;
      x2_prev = x2;
      y2_prev = y2;

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

void draw_line(int x0, int y0, int x1, int y1, short int color) {
  // lines should be only vertical or horizontal
  if (x1 > x0 && y0 == y1) {
    // which means it's horizontal line
    for (int i = x0; i < x1; i++) {
      if (y0 - 1 > 0) {
        plot_pixel(i, y0 - 1, color);
      }
      plot_pixel(i, y0, color);
      plot_pixel(i, y0 + 1, color);
    }
  }
  if (x1 == x0 && y0 < y1) {
    // which means it's vertical line
    for (int i = y0; i < y1; i++) {
      if (x0 - 1 > 0) {
        plot_pixel(x0 - 1, i, color);
      }
      plot_pixel(x0, i, color);
      plot_pixel(x0 + 1, i, color);
    }
  }
}

void swap(int *x, int *y) {
  int temp = *x;
  *x = *y;
  *y = temp;
}

void draw_frame() {
  draw_line(0, 0, 0, 240, 0x07e0);
  draw_line(0, 0, 320, 0, 0x07e0);
  draw_line(319, 0, 319, 240, 0x07e0);
  draw_line(0, 239, 319, 239, 0x07e0);
}

void draw_entrance_exit() {
  draw_line(0, 200, 0, 220, 0x0);
  // draw_line(0,0, 320, 0 , 0x0) ;
  draw_line(319, 200, 319, 220, 0x0);
  // draw_line(0,239, 319, 239 , 0x0) ;
}

void draw_maze_level1() {
  draw_line(80, 2, 80, 150, 0xffff);
  draw_line(160, 80, 160, 238, 0xffff);
  draw_line(160, 160, 200, 160, 0xffff);
  draw_line(240, 80, 240, 238, 0xffff);
  draw_line(260, 160, 319, 160, 0xffff);
  draw_line(40, 180, 120, 180, 0xffff);
}

void draw_maze_level2() {
  draw_line(2, 120, 100, 120, 0xffff);
  draw_line(100, 140, 100, 160, 0xffff);
  draw_line(100, 160, 120, 160, 0xffff);
  draw_line(120, 2, 120, 80, 0xffff);
  draw_line(120, 80, 240, 80, 0xffff);
  draw_line(240, 80, 240, 100, 0xffff);
  draw_line(120, 100, 120, 160, 0xffff);
  draw_line(120, 100, 220, 100, 0xffff);
  draw_line(120, 140, 240, 140, 0xffff);
  draw_line(160, 160, 160, 238, 0xffff);
  draw_line(200, 160, 300, 160, 0xffff);
  draw_line(300, 80, 300, 160, 0xffff);
  draw_line(300, 100, 329, 100, 0xffff);
  draw_line(20, 150, 100, 150, 0xffff);
  draw_line(60, 150, 60, 210, 0xffff);
  draw_line(60, 200, 140, 200, 0xffff);
  draw_line(100, 60, 100, 120, 0xffff);
  draw_line(50, 80, 100, 80, 0xffff);
  draw_line(50, 40, 50, 80, 0xffff);
  draw_line(240, 2, 240, 60, 0xffff);
  draw_line(240, 40, 260, 80, 0xffff);
  draw_line(160, 30, 300, 30, 0xffff);
  draw_line(240, 160, 240, 220, 0xffff);
  draw_line(180, 200, 300, 200, 0xffff);
  draw_line(270, 80, 270, 140, 0xffff);
}

void draw_menu() {
  const short int(*image_colour)[320] = menu;
  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 320; x++) {
      plot_pixel(x, y, image_colour[y][x]);
    }
  }
}

void draw_maze() {
  const short int(*image_colour)[320] = maze_pacman;
  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 320; x++) {
      plot_pixel(x, y, image_colour[y][x]);
    }
  }
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

bool is_blocked(int x, int y) {
  const short int wall_color =
      0x6000;  // Assuming black is the color for open paths
  // Loop over the character's bounding box
  for (int i = 0; i < CHARACTER_HEIGHT; i++) {
    for (int j = 0; j < CHARACTER_WIDTH; j++) {
      int pixel_x = x + j;
      int pixel_y = y + i;
      // Check if within screen bounds to avoid accessing out of bounds
      if (pixel_x < 0 || pixel_x >= SCREEN_WIDTH || pixel_y < 0 ||
          pixel_y >= SCREEN_HEIGHT) {
        return true;  // Treat out-of-bounds as blocked
      }
      if (maze_pacman[pixel_y][pixel_x] > wall_color) {
        return true;  // Blocked by a wall
      }
    }
  }
  return false;  // Not blocked
}

void move_player1(uint8_t scan_code, int *x, int *y) {
  int new_x = *x, new_y = *y;

  if (scan_code == 0x1C) {  // 'A' pressed, move left
    new_x -= 3;
  } else if (scan_code == 0x1D) {  // 'W' pressed, move up
    new_y -= 3;
  } else if (scan_code == 0x1B) {  // 'S' pressed, move down
    new_y += 3;
  } else if (scan_code == 0x23) {  // 'D' pressed, move right
    new_x += 3;
  }

  if (!is_blocked(new_x, new_y)) {
    *x = new_x;
    *y = new_y;
    check_and_correct_boundaries(x, y);
  }
}

void move_player2(uint8_t scan_code, int *x, int *y) {
  int new_x = *x, new_y = *y;

  if (scan_code == 0x6B) {  // 'A' pressed, move left
    new_x -= 3;
  } else if (scan_code == 0x75) {  // 'W' pressed, move up
    new_y -= 3;
  } else if (scan_code == 0x72) {  // 'S' pressed, move down
    new_y += 3;
  } else if (scan_code == 0x74) {  // 'D' pressed, move right
    new_x += 3;
  }

  if (!is_blocked(new_x, new_y)) {
    *x = new_x;
    *y = new_y;
    check_and_correct_boundaries(x, y);
  }
}

// void draw_character(int top_left_x, int top_left_y, int color) {
//   for (int y = 0; y < 15; ++y) {
//     for (int x = 0; x < 15; ++x) {
//       plot_pixel(top_left_x + x, top_left_y + y, character[y][x]);
//     }
//   }
// }

void draw_character(int top_left_x, int top_left_y, unsigned int color,
                    int num) {
  // If the color is background, we're erasing the character
  bool erase = (color == COLOR_BACKGROUND);

  for (int y = 0; y < 15; ++y) {
    for (int x = 0; x < 15; ++x) {
      // Draw or erase depending on the color
      unsigned int draw_color = erase ? COLOR_BACKGROUND : character[num][y][x];
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

void clear_players_keep_maze() {
  draw_frame();
  draw_entrance_exit();
  draw_maze_level1();
}
#include <string.h>  // Include the header for memcpy function
#include <time.h>    // Inlcude for random destination generation

#include "audio.c"
#include "characters.c"
#include "end_screen.c"
#include "globals.h"
#include "maze2.c"
#include "maze3.c"
#include "menu.c"

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

int main(void) {
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  volatile int *SW_ptr = (int *)0xff200040;
  volatile int *LEDR_ptr = (int *)0xff200000;
  srand(time(NULL));
  bool gameRunning = true;
  while (gameRunning) {
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    *(pixel_ctrl_ptr + 1) = (int)Buffer1;  // Set front buffer
    wait_for_vsync();
    pixel_buffer_start =
        *pixel_ctrl_ptr;  // Use the current front buffer to clear screen
    draw_screen(menu);
    *(pixel_ctrl_ptr + 1) = (int)Buffer2;        // Set back buffer
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // Draw on back buffer
    // uint16_t chosen_maze[240][320] = {0};

    // decide map level 1 to 3
    while (1) {
      int value = *SW_ptr;
      if (value == 0b1) {
        *LEDR_ptr = value;  // Indicate the selected maze on LEDs
        // memcpy(chosen_maze, maze2, sizeof(chosen_maze));
        clear_screen();      // Clears the back buffer
        draw_screen(maze2);  // Draws the maze onto the back buffer
        wait_for_vsync();    // Wait for the vertical synchronization
        *(pixel_ctrl_ptr + 1) = pixel_buffer_start;
        pixel_buffer_start = *(pixel_ctrl_ptr);
        break;
      }
      if (value == 0b10) {
        *LEDR_ptr = value;  // Indicate the selected maze on LEDs
        // memcpy(chosen_maze, maze3, sizeof(chosen_maze));
        clear_screen();      // Clears the back buffer
        draw_screen(maze3);  // Draws the maze onto the back buffer
        wait_for_vsync();    // Wait for the vertical synchronization
        *(pixel_ctrl_ptr + 1) = pixel_buffer_start;
        pixel_buffer_start = *(pixel_ctrl_ptr);
        break;
      }
      if (value == 0b100) {
        *LEDR_ptr = value;  // Indicate the selected maze on LEDs
        // memcpy(chosen_maze, maze3, sizeof(chosen_maze));
        clear_screen();  // Clears the back buffer
        // draw_screen(chosen_maze);  // Draws the maze onto the back buffer
        wait_for_vsync();  // Wait for the vertical synchronization
        *(pixel_ctrl_ptr + 1) = pixel_buffer_start;
        pixel_buffer_start = *(pixel_ctrl_ptr);
        break;
      }
    }

    volatile int *PS2_ptr = (int *)PS2_BASE;  // PS/2 base address
    int buffer = 0;                           // Start with Buffer1
    unsigned long last_time = 0;  // Last time we updated the countdown

    setup_timer();          // Initialize the timer for countdown
    uint32_t counter = 45;  // Start counting from 9

    int PS2_data, RVALID;
    char byte3 = 0;

    int x1 = 17;  // Player 1 entrance
    int y1 = 205;
    int x2 = 293;  // Player 2 entrance
    int y2 = 205;

    // PS/2 keyboard reset
    *PS2_ptr = 0xFF;  // Reset

    // Assuming the initial position of the box is in the center
    int x1_prev = x1, y1_prev = y1;  // Position from the last frame
    int x1_old = x1, y1_old = y1;    // Position from two frames ago
    int x2_prev = x2, y2_prev = y2;  // Position from the last frame
    int x2_old = x2, y2_old = y2;    // Position from two frames ago

    // Destination dest = create_dynamic_destination(chosen_maze);
    // draw_destination(dest);  // Draw the initial destination
    Destination dest;
    if (value == 0b1) {
      dest = create_dynamic_destination_2();

    } else {
      dest = create_dynamic_destination_3();
    }
    draw_destination(dest);  // Draw the initial destination

    while (1) {
      display_hex(counter);

      // Check for PS/2 keyboard inputs
      PS2_data = *PS2_ptr;         // Read the Data register in the PS/2 port
      RVALID = PS2_data & 0x8000;  // Extract the RVALID field
      // Inside the main loop, before clear_screen()
      if (RVALID) {
        value = *SW_ptr;
        if (value == 0b1) {
          // Move player based on the current input
          //   Destination dest = create_dynamic_destination_2();
          //   draw_destination(dest);  // Draw the initial destination
          byte3 = PS2_data & 0xFF;
          move_player1_2(byte3, &x1, &y1);  // This updates the current position

          move_player2_2(byte3, &x2, &y2);  // This updates the current position

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
          *(pixel_ctrl_ptr + 1) =
              pixel_buffer_start;  // Update back buffer address

        } else {
          //   Destination dest = create_dynamic_destination_3();
          //   draw_destination(dest);  // Draw the initial destination
          // Move player based on the current input
          byte3 = PS2_data & 0xFF;
          move_player1_3(byte3, &x1, &y1);  // This updates the current position

          move_player2_3(byte3, &x2, &y2);  // This updates the current position

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
          *(pixel_ctrl_ptr + 1) =
              pixel_buffer_start;  // Update back buffer address
        }
      }
      if (has_caught(x1, y1, x2, y2) || counter == 1) {
        audio_play_for_lose(samples_2, samples_n_2);
        break;
      }

      if (has_reached_destination(x2, y2, dest)) {
        // audio_play_for_win(samples_1, samples_n_1) ;
        draw_destination(dest);
        continue;
      }

      if (has_reached_destination(x1, y1, dest)) {
        audio_play_for_win(samples_1, samples_n_1);
        clear_destination(dest);
        break;
      }

      if (TIMER_BASE[0] & 0x1) {
        TIMER_BASE[0] = 0;  // Reset TO bit to count another second
        counter--;          // Decrement the main game counter every second

        // Check if the 10-second interval has passed
        if (counter % 10 == 0 && counter != 0) {
          // Clear the old destination
          clear_destination(dest);

          // Generate a new dynamic destination and draw it
          if (value == 0b1) {
            dest = create_dynamic_destination_2();
          } else {
            dest = create_dynamic_destination_3();
          }
          draw_destination(dest);
        }

        if (counter == 0) {
          // Counter has reached 0, time is up
          break;  // Exit loop when counter reaches 0
        }
      }
    }
    wait_for_vsync();
    *(pixel_ctrl_ptr + 1) = pixel_buffer_start;  // Update back buffer address

    if (has_reached_destination(x1, y1, dest)) {
      draw_screen(player1win);
      wait_for_vsync();
    } else {
      draw_screen(player2win);
      wait_for_vsync();
    }

    bool restartGame = false;
    while (!restartGame) {
      if (*PS2_ptr & 0x8000) {    // Check if data is available in the buffer
        byte3 = *PS2_ptr & 0xFF;  // Read the data from the buffer
        if (byte3 == 0x29) {      // Check if it's the space bar scancode
          restartGame = true;     // Set the flag to exit this loop
        }
      }
      wait_for_vsync();
    }
    clear_hex_display();
    clear_screen();
  }
  return 0;
}

void draw_screen(const uint16_t draw[240][320]) {
  const uint16_t(*image_colour)[320] = draw;
  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 320; x++) {
      plot_pixel(x, y, image_colour[y][x]);
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
    *x = SCREEN_WIDTH - 15 - 1;
  }

  if (*y < 0) {
    *y = 0;
  } else if (*y + 15 >= SCREEN_HEIGHT) {
    *y = SCREEN_HEIGHT - 15 - 1;
  }
}

bool is_blocked2(int x, int y) {
  const short int wall_color = 0x0000;
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
      if (maze2[pixel_y][pixel_x] > 0xaaaa) {
        return true;  // Blocked by a wall
      }
    }
  }
  return false;  // Not blocked
}

bool is_blocked3(int x, int y) {
  const short int wall_color = 0x0000;
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
      if (maze3[pixel_y][pixel_x] > wall_color) {
        return true;  // Blocked by a wall
      }
    }
  }
  return false;  // Not blocked
}

void move_player1_2(uint8_t scan_code, int *x, int *y) {
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

  if (!is_blocked2(new_x, new_y)) {
    *x = new_x;
    *y = new_y;
    check_and_correct_boundaries(x, y);
  }
}

void move_player2_2(uint8_t scan_code, int *x, int *y) {
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

  if (!is_blocked2(new_x, new_y)) {
    *x = new_x;
    *y = new_y;
    check_and_correct_boundaries(x, y);
  }
}
void move_player1_3(uint8_t scan_code, int *x, int *y) {
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

  if (!is_blocked3(new_x, new_y)) {
    *x = new_x;
    *y = new_y;
    check_and_correct_boundaries(x, y);
  }
}

void move_player2_3(uint8_t scan_code, int *x, int *y) {
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

  if (!is_blocked3(new_x, new_y)) {
    *x = new_x;
    *y = new_y;
    check_and_correct_boundaries(x, y);
  }
}
bool has_caught(int p1_x, int p1_y, int p2_x, int p2_y) {
  // Define the bounds of player 1
  int p1_top = p1_y;
  int p1_bottom = p1_y + 11;
  int p1_left = p1_x;
  int p1_right = p1_x + 11;

  // Define the bounds of player 2
  int p2_top = p2_y;
  int p2_bottom = p2_y + 11;
  int p2_left = p2_x;
  int p2_right = p2_x + 11;

  // Check for overlap between players
  bool horizontal_overlap = (p1_left < p2_right) && (p1_right > p2_left);
  bool vertical_overlap = (p1_top < p2_bottom) && (p1_bottom > p2_top);

  return horizontal_overlap && vertical_overlap;
}

void draw_character(int top_left_x, int top_left_y, unsigned int color,
                    int num) {
  // If the color is background, we're erasing the character
  bool erase = (color == COLOR_BACKGROUND);

  for (int y = 0; y < CHARACTER_HEIGHT; ++y) {
    for (int x = 0; x < CHARACTER_WIDTH; ++x) {
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

// Function to create a dynamic destination
Destination create_dynamic_destination_2() {
  Destination dest;
  bool valid = false;

  while (!valid) {
    // Generate a random top-left corner for the destination
    dest.x = rand() % (SCREEN_WIDTH - 50 - DESTINATION_SIZE) + 50;
    dest.y = rand() % (SCREEN_HEIGHT - 50 - DESTINATION_SIZE) + 50;

    // Assume the destination is valid until proven otherwise
    valid = true;

    // Check if the generated position is valid
    for (int y = -1; y < DESTINATION_SIZE + 5; y++) {
      for (int x = -1; x < DESTINATION_SIZE + 5; x++) {
        if (maze2[dest.y - 5 + y][dest.x - 5 + x] !=
            0x0000) {     // If not black, it's a wall
          valid = false;  // Destination overlaps with a wall, so it's not valid
        }
      }
    }
  }
  return dest;
}

// Function to create a dynamic destination
Destination create_dynamic_destination_3() {
  Destination dest;
  bool valid = false;

  while (!valid) {
    // Generate a random top-left corner for the destination
    dest.x = rand() % (SCREEN_WIDTH - DESTINATION_SIZE);
    dest.y = rand() % (SCREEN_HEIGHT - DESTINATION_SIZE);

    // Assume the destination is valid until proven otherwise
    valid = true;

    // Check if the generated position is valid
    for (int y = -1; y < DESTINATION_SIZE + 5; y++) {
      for (int x = -1; x < DESTINATION_SIZE + 5; x++) {
        if (maze3[dest.y - 5 + y][dest.x - 5 + x] !=
            0x0000) {     // If not black, it's a wall
          valid = false;  // Destination overlaps with a wall, so it's not valid
        }
      }
    }
  }
  return dest;
}

// Function to draw the destination on the screen
void draw_destination(Destination dest) {
  for (int y = 0; y < DESTINATION_SIZE; y++) {
    for (int x = 0; x < DESTINATION_SIZE; x++) {
      // Make sure the coordinates are within the screen bounds
      if ((dest.x + x) < SCREEN_WIDTH && (dest.y + y) < SCREEN_HEIGHT) {
        plot_pixel(dest.x + x, dest.y + y, COLOR_GREEN);
      }
    }
  }
}

bool has_reached_destination(int p1_x, int p1_y, Destination dest) {
  // Define the bounds of player 1
  int p1_top = p1_y;
  int p1_bottom = p1_y + 11;
  int p1_left = p1_x;
  int p1_right = p1_x + 11;

  // Define the bounds of the destination
  int dest_top = dest.y;
  int dest_bottom = dest.y + DESTINATION_SIZE;
  int dest_left = dest.x;
  int dest_right = dest.x + DESTINATION_SIZE;

  // Check for overlap between player 1 and the destination
  bool horizontal_overlap = (p1_left < dest_right) && (p1_right > dest_left);
  bool vertical_overlap = (p1_top < dest_bottom) && (p1_bottom > dest_top);

  return horizontal_overlap && vertical_overlap;
}

void clear_destination(Destination dest) {
  for (int y = 0; y < DESTINATION_SIZE; y++) {
    for (int x = 0; x < DESTINATION_SIZE; x++) {
      plot_pixel(dest.x + x, dest.y + y, COLOR_BACKGROUND);
    }
  }
}

void clear_hex_display(void) {
  volatile int *hex_display_ptr = (int *)HEX_BASE;
  *hex_display_ptr =
      0x0000;  // Turn off all segments for the first 4 HEX displays
}

void audio_play_for_win(int *samples, int sample_n_1) {
  int count = 0;

  audiop->control = 0x8;  // clear the output FIFOs
  audiop->control = 0x0;  // resume input conversion
  while (count < sample_n_1) {
    // output data if there is space in the output FIFOs
    if ((audiop->wsrc != 0) && (audiop->wslc != 0)) {
      audiop->ldata = samples[count];
      audiop->rdata = samples[count++];
    }
  }
}

void audio_play_for_lose(int *samples, int sample_n_1) {
  int count = 0;

  audiop->control = 0x8;  // clear the output FIFOs
  audiop->control = 0x0;  // resume input conversion
  while (count < sample_n_1) {
    // output data if there is space in the output FIFOs
    if ((audiop->wsrc != 0) && (audiop->wslc != 0)) {
      audiop->ldata = samples[count];
      audiop->rdata = samples[count++];
    }
  }
}

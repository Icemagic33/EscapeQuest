#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  COLOR_BLACK = 0,
  COLOR_WHITE = 0xFFFF,
  COLOR_RED = 0xF800,
} Color;

typedef struct {
  int width;
  int height;
  Color **pixels;
} Maze;

Maze *create_maze(int width, int height) {
  Maze *maze = malloc(sizeof(Maze));
  maze->width = width;
  maze->height = height;
  maze->pixels = malloc(height * sizeof(Color *));
  for (int i = 0; i < height; i++) {
    maze->pixels[i] = malloc(width * sizeof(Color));
    for (int j = 0; j < width; j++) {
      maze->pixels[i][j] = COLOR_BLACK;  // Initialize the maze with black color
    }
  }
  for (int i = 20; i < height / 2; i++) {
    for (int j = 40; j < width / 2; j++) {
      maze->pixels[i][j] = COLOR_WHITE;
    }
    return maze;
  }
}
void free_maze(Maze *maze) {
  for (int i = 0; i < maze->height; i++) {
    free(maze->pixels[i]);
  }
  free(maze->pixels);
  free(maze);
}

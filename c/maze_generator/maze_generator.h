#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H

#include <stdbool.h> // for bool
#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint8_t

#include "../bmp/bmp.h"
#include "../stack/stack.h"

extern struct RGBTriple MAZE_START_COLOR; // default {0,255,0}
extern struct RGBTriple MAZE_END_COLOR;   // default {255,0,0}
extern struct RGBTriple MAZE_WALL_COLOR;  // default {0,0,0}
extern struct RGBTriple MAZE_BG_COLOR;    // default {255,255,255}

struct Point {
    int32_t x;
    int32_t y;
};

enum Direction {
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3
};

struct Cell {
    struct Point position; // pixel coordinates of the cell
    bool visited; // true if the cell has been visited
    bool walls[4]; // {UP | RIGHT | DOWN | LEFT}
};

struct Maze {
    size_t width; // in cells
    size_t height; // in cells
    uint32_t cell_size; // pixel size of each cell (e.g. 4 for 4×4 px)
    struct Point start; // starting cell coords
    struct Point end; // ending cell coords
    uint32_t wall_thickness; // pixel thickness of walls (e.g. 1 for 1 px)
    struct Cell **cells; // 2D array [row][col]
};

struct Maze* maze_create(size_t width, size_t height, uint32_t cell_size, struct Point start, struct Point end, uint32_t wall_thickness); // Allocate and initialize a new Maze (all walls present, unvisited)

void maze_free(struct Maze *m); // Free all memory associated with a Maze

void maze_reset(struct Maze *m); // Reset maze state: mark all cells unvisited and restore all walls

void maze_generate_dfs(struct Maze *m); // Generate the maze using Depth-First Search (recursive backtracking)

bool maze_has_unvisited_neighbor(const struct Maze *m, const struct Cell *c, enum Direction *out_dir); // Check if a cell has at least one unvisited neighbor

struct Cell* maze_get_neighbor(const struct Maze *m, const struct Cell *c, enum Direction dir); // Get pointer to the neighboring cell in the given direction, or NULL if out of bounds

void maze_remove_wall(struct Cell *c1, struct Cell *c2, enum Direction dir); // Remove the wall between two neighboring cells

void maze_render_to_bmp(struct Maze *m, struct BmpImage*); // Draw the maze to a BMP file

void maze_render_cell(const struct Maze *m, struct BmpImage *img, const struct Cell *cell); // Draw a cell at its pixel coordinates

#endif // MAZE_GENERATOR_H
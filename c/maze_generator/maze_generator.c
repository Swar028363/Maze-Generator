#include "maze_generator.h"

#include <stdio.h>  // for FILE, fopen, fclose, fwrite
#include <stdlib.h> // for malloc, free
#include <math.h>  // for floor
#include <string.h> // for memset

typedef struct RGBTriple RGBTriple;
typedef struct BmpImage BmpImage;
typedef struct BmpFileHeader BmpFileHeader;
typedef struct BmpInfoHeader BmpInfoHeader;
typedef struct Cell Cell;
typedef struct Point Point;
typedef struct Maze Maze;
typedef enum Direction Direction;
typedef struct Stack Stack;
typedef struct StackNode StackNode;

RGBTriple MAZE_START_COLOR = {0, 255, 0}; // default start color
RGBTriple MAZE_END_COLOR = {255, 0, 0};   // default end color
RGBTriple MAZE_WALL_COLOR = {0, 0, 0};    // default wall color
RGBTriple MAZE_BG_COLOR = {255, 255, 255}; // default background color

struct Maze* maze_create(size_t width, size_t height, uint32_t cell_size, struct Point start, struct Point end, uint32_t wall_thickness) {
    if (width == 0 || height == 0 || cell_size <= 0 || wall_thickness < 0) {
        return NULL; // Invalid parameters
    }

    if (start.x < 0 || start.x >= width || start.y < 0 || start.y >= height) {
        return NULL; // Invalid start coordinates
    }

    if (end.x < 0 || end.x >= width || end.y < 0 || end.y >= height) {
        return NULL; // Invalid end coordinates
    }

    if (wall_thickness >= floor(cell_size / 2)) {
        wall_thickness = floor(cell_size / 2); // Ensure wall thickness is less than half the cell size
    }

    if (wall_thickness < 1) {
        wall_thickness = 1; // Minimum wall thickness
    }

    struct Maze *maze = malloc(sizeof(struct Maze));
    if (!maze) {
        return NULL; // Memory allocation failed
    }

    maze->width = width;
    maze->height = height;
    maze->cell_size = cell_size;
    maze->start = start;
    maze->end = end;
    maze->wall_thickness = wall_thickness;
    maze->cells = malloc(height * sizeof(struct Cell *));
    if (!maze->cells) {
        free(maze);
        return NULL; // Memory allocation failed
    }

    struct Cell *block = malloc(width * height * sizeof(struct Cell));
    if (!block) {
        free(maze->cells);
        free(maze);
        return NULL; // Memory allocation failed
    }

    for (size_t y = 0; y < height; ++y) {
        maze->cells[y] = &block[y * width];
        for (size_t x = 0; x < width; ++x) {
            struct Cell *cell = &maze->cells[y][x];
            cell->position.x = x;
            cell->position.y = y;
            cell->visited = false;
            memset(cell->walls, true, sizeof(cell->walls)); // All walls present
        }
    }
    maze->cells[start.y][start.x].visited = true; // Mark start cell as visited
    return maze;

}

void maze_free(struct Maze *m) {
    if (m) {
        free(m->cells[0]); // Free the block of cells
        free(m->cells);    // Free the array of pointers
        free(m);          // Free the maze structure
    }
}

void maze_reset(struct Maze *m) {
    if (!m) return;
    for (size_t y = 0; y < m->height; ++y) {
        for (size_t x = 0; x < m->width; ++x) {
            struct Cell *c = &m->cells[y][x];
            c->visited = false; // Reset visited status
            memset(c->walls, true, sizeof(c->walls)); // Restore all walls
        }
    }
    // Mark start cell as visited
    if (m->start.x >= 0 && m->start.x < (int)m->width &&
        m->start.y >= 0 && m->start.y < (int)m->height) {
        m->cells[m->start.y][m->start.x].visited = true;
    }
}

void maze_generate_dfs(struct Maze *m) {
    if (!m || !m->cells) return;

    // Reset the maze to initial state
    maze_reset(m);

    // Create a stack to hold pointers to cells
    Stack *stack = createStack(-1); // -1 for unlimited capacity
    if (!stack) return; // Stack creation failed

    // Get the starting cell
    struct Cell *start_cell = &m->cells[m->start.y][m->start.x];
    start_cell->visited = true;
    push(stack, start_cell);

    while (!isEmpty(stack)) {
        struct Cell *current = (struct Cell *)peek(stack);
        Direction dir;

        if (maze_has_unvisited_neighbor(m, current, &dir)) {
            // Get the neighboring cell in the chosen direction
            struct Cell *neighbor = maze_get_neighbor(m, current, dir);
            if (neighbor) {
                // Remove the wall between current and neighbor
                maze_remove_wall(current, neighbor, dir);
                neighbor->visited = true;
                push(stack, neighbor);
            }
        } else {
            // Backtrack
            pop(stack);
        }
    }

    // Clean up
    freeStack(stack, NULL);
}

void shuffle_directions(Direction *dir, int n)
{
    for (int i = n - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        Direction temp = dir[i];
        dir[i] = dir[j];
        dir[j] = temp;
    }
}

bool maze_has_unvisited_neighbor(const struct Maze *m, const struct Cell *c, enum Direction *out_dir) {
    if (!m || !m->cells || !c) return false;

    Direction dirs[4] = {UP, RIGHT, DOWN, LEFT};
    shuffle_directions(dirs, 4);

    for (int i = 0; i < 4; ++i) {
        struct Cell *neighbor = maze_get_neighbor(m, c, dirs[i]);
        if (neighbor && !neighbor->visited) {
            if (out_dir) {
                *out_dir = dirs[i];
            }
            return true;
        }
    }
    return false;
}

struct Cell* maze_get_neighbor(const struct Maze *m, const struct Cell *c, enum Direction dir) {
    if (!m || !m->cells || !c) return NULL;

    int dx = 0, dy = 0;
    switch (dir) {
        case UP:    dy = -1; break;
        case DOWN:  dy = +1; break;
        case LEFT:  dx = -1; break;
        case RIGHT: dx = +1; break;
        default:    return NULL; // invalid enum value
    }

    int nx = c->position.x + dx;
    int ny = c->position.y + dy;
    if (nx < 0 || nx >= (int)m->width || ny < 0 || ny >= (int)m->height) {
        return NULL; // Out of bounds
    }

    return &m->cells[ny][nx];
}

void maze_remove_wall(struct Cell *c1, struct Cell *c2, enum Direction dir) {
    if (!c1 || !c2) return;

    switch (dir) {
        case UP:
            c1->walls[UP] = false;
            c2->walls[DOWN] = false;
            break;
        case DOWN:
            c1->walls[DOWN] = false;
            c2->walls[UP] = false;
            break;
        case LEFT:
            c1->walls[LEFT] = false;
            c2->walls[RIGHT] = false;
            break;
        case RIGHT:
            c1->walls[RIGHT] = false;
            c2->walls[LEFT] = false;
            break;
        default:
            break; // invalid enum value
    }
}


void maze_color_start_end(const Maze *m, BmpImage *img) {
    if (!m || !img) return;

    // shorthand
    int cs = m->cell_size, wt = m->wall_thickness;
    int sx = m->start.x, sy = m->start.y;
    int ex = m->end.x,   ey = m->end.y;

    // Compute pixel bounds for an inset rectangle inside the walls
    int sx0 = sx*cs + wt,  sy0 = sy*cs + wt;
    int sx1 = (sx+1)*cs - 1 -wt, sy1 = (sy+1)*cs - 1 - wt;

    // Paint start region
    for (int y = sy0; y <= sy1; ++y) {
        for (int x = sx0; x <= sx1; ++x) {
            bmp_set_pixel(img, x, y, MAZE_START_COLOR);
        }
    }

    // Compute for end cell
    int ex0 = ex*cs + wt,  ey0 = ey*cs + wt;
    int ex1 = (ex+1)*cs - 1 - wt, ey1 = (ey+1)*cs - 1 - wt;

    // Paint end region
    for (int y = ey0; y <= ey1; ++y) {
        for (int x = ex0; x <= ex1; ++x) {
            bmp_set_pixel(img, x, y, MAZE_END_COLOR);
        }
    }
}


void maze_render_to_bmp(struct Maze *m, const char *filename) {
    if (!m || !filename) return;

    BmpImage* img = bmp_create(m->width * m->cell_size, m->height * m->cell_size);
    if (!img) return; // Image creation failed

    for (size_t y = 0; y < img->infoHeader.biHeight; ++y) {
        for (size_t x = 0; x < img->infoHeader.biWidth; ++x) {
            bmp_set_pixel(img, x, y, MAZE_BG_COLOR);
        }
    }

    for (size_t cy = 0; cy < m->height; ++cy) {
        for (size_t cx = 0; cx < m->width; ++cx) {
            maze_render_cell(m, img, &m->cells[cy][cx]);
        }
    }

    // Color start and end cells
    maze_color_start_end(m, img);

    bmp_save(filename, img);
}

void maze_render_cell(const struct Maze *m, struct BmpImage *img, const struct Cell *cell) {
    int cs = m->cell_size, T = m->wall_thickness;
    int x0 = cell->position.x * cs;
    int y0 = cell->position.y * cs;
    int x1 = x0 + cs - 1;
    int y1 = y0 + cs - 1;

    // Fill background
    for (int y = y0; y <= y1; ++y){
        for (int x = x0; x <= x1; ++x) {
            bmp_set_pixel(img, x, y, MAZE_BG_COLOR);
        }
    }
    
    // Draw each wall as a stripe of thickness T
    if (cell->walls[UP]) {
        for (int t = 0; t < T; ++t) {
            for (int x = x0; x <= x1; ++x) {
                bmp_set_pixel(img, x, y0 + t, MAZE_WALL_COLOR);
            }
        }
    }

    if (cell->walls[DOWN]) {
        for (int t = 0; t < T; ++t) {
            for (int x = x0; x <= x1; ++x) {
                 bmp_set_pixel(img, x, y1 - t, MAZE_WALL_COLOR);
            }
        }
    }

    if (cell->walls[LEFT]) {
        for (int t = 0; t < T; ++t) {
            for (int y = y0; y <= y1; ++y) {
                bmp_set_pixel(img, x0 + t, y, MAZE_WALL_COLOR);
            }
        }
    }

    if (cell->walls[RIGHT]) {
        for (int t = 0; t < T; ++t) {
            for (int y = y0; y <= y1; ++y) {
                bmp_set_pixel(img, x1 - t, y, MAZE_WALL_COLOR);
            }
        }
    }
}
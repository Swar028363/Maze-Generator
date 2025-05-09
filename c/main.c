#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./maze_generator/maze_generator.h"

typedef struct Point Point;
typedef struct Maze Maze;
typedef struct RGBTriple RGBTriple;

int main() {
    size_t width = 20, height = 20;
    int    cell_size = 10;
    int    wall_thickness = 1;
    Point  start = {0,0};
    Point  end = {19,19};

    // {blue, green, red}
    MAZE_BG_COLOR = (RGBTriple){200, 200, 200};
    MAZE_WALL_COLOR = (RGBTriple){50, 50, 50};
    MAZE_START_COLOR = (RGBTriple){255, 255, 0};
    MAZE_END_COLOR = (RGBTriple){0, 0, 255};
    

    Maze *maze = maze_create(width, height, cell_size, start, end, wall_thickness);
    if (!maze) {
        fprintf(stderr, "Failed to create maze\n");
        return 1;
    }

    srand((unsigned)time(NULL));

    maze_generate_dfs(maze);

    maze_render_to_bmp(maze, "output/maze.bmp");

    maze_free(maze);

    return 0;
}

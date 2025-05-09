#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#include "maze_generator/maze_generator.h"
#include "bmp/bmp.h"

typedef struct Point   Point;
typedef struct RGBTriple RGBTriple;
typedef struct Maze    Maze;
typedef struct BmpImage BmpImage;

/* clamp integer to [0,255] */
static int clamp255(int v) {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return v;
}

/* check if `str` ends with `suffix` */
static int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;
    size_t n = strlen(str), m = strlen(suffix);
    if (m > n) return 0;
    return strncmp(str + n - m, suffix, m) == 0;
}

/* print usage to stderr; doesn't exit() so caller controls exit code */
static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [OPTIONS]\n\n"
        "Options:\n"
        "  -f, --file <path>         Output BMP filename\n"
        "  -d, --dims W H            Maze dimensions in cells (default: 20 20)\n"
        "  -c, --cell N              Cell size in pixels (default: 10)\n"
        "  -w, --wall N              Wall thickness in pixels (default: 1)\n"
        "  -s, --start X Y           Start cell coordinates (default: 0 0)\n"
        "  -e, --end X Y             End cell coordinates (default: W-1 H-1)\n"
        "      --bgc R G B           Background color (default: 255 255 255)\n"
        "      --wc R G B            Wall color (default: 0 0 0)\n"
        "      --sc R G B            Start cell color (default: 0 255 0)\n"
        "      --ec R G B            End cell color (default: 255 0 0)\n"
        "      --seed SEED           RNG seed for reproducible output\n"
        "  -v, --verbose             Print debug information\n"
        "  -h, --help                Show this help and exit\n"
        "      --version             Show version and exit\n",
        prog
    );
}

/* simple version string */
static const char *VERSION = "1.0.0";

/* Helper: compute milliseconds between two timespecs */
static double diff_ms(const struct timespec *start, const struct timespec *end) {
    double sec  = (double)(end->tv_sec  - start->tv_sec);
    double nsec = (double)(end->tv_nsec - start->tv_nsec);
    return sec * 1e3 + nsec / 1e6;
}

int main(int argc, char *argv[]) {
    /* defaults */
    size_t     width     = 20, height    = 20;
    int        cell_size = 10;
    int        wall_th   = 1;
    Point      start     = {0, 0};
    Point      endp      = {0, 0};

    RGBTriple  bgc = {255,255,255},
               wc  = {0,0,0},
               sc  = {0,255,0},
               ec  = {255,0,0};

    unsigned long seed = (unsigned long) time(NULL);
    int verbose = 0;
    char out_filename[256] = {0};

    /* long options table */
    static struct option long_opts[] = {
        {"file",    required_argument, 0, 'f'},
        {"dims",    required_argument, 0, 'd'},
        {"cell",    required_argument, 0, 'c'},
        {"wall",    required_argument, 0, 'w'},
        {"start",   required_argument, 0, 's'},
        {"end",     required_argument, 0, 'e'},
        {"bgc",     required_argument, 0,  1 },
        {"wc",      required_argument, 0,  2 },
        {"sc",      required_argument, 0,  3 },
        {"ec",      required_argument, 0,  4 },
        {"seed",    required_argument, 0,  5 },
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0,  6 },
        {0,0,0,0}
    };

    int opt, idx;
    while ((opt = getopt_long(argc, argv, "f:d:c:w:s:e:vh", long_opts, &idx)) != -1) {
        switch (opt) {
            case 'f':
                strncpy(out_filename, optarg, sizeof out_filename - 1);
                break;
            case 'd':
                if (optind < argc) {
                    width  = strtoul(optarg, NULL, 10);
                    height = strtoul(argv[optind++], NULL, 10);
                }
                break;
            case 'c': cell_size = atoi(optarg); break;
            case 'w': wall_th   = atoi(optarg); break;
            case 's':
                if (optind < argc) {
                    start.x = atoi(optarg);
                    start.y = atoi(argv[optind++]);
                }
                break;
            case 'e':
                if (optind < argc) {
                    endp.x = atoi(optarg);
                    endp.y = atoi(argv[optind++]);
                }
                break;
            case 1:  /* --bgc */
                if (optind + 1 >= argc) {
                    fprintf(stderr, "Error: --bgc requires three arguments\n");
                    return EXIT_FAILURE;
                }
                bgc.rgbtRed   = clamp255(atoi(optarg));
                bgc.rgbtGreen = clamp255(atoi(argv[optind++]));
                bgc.rgbtBlue  = clamp255(atoi(argv[optind++]));
                break;
            case 2:  /* --wc */
                if (optind + 1 >= argc) {
                    fprintf(stderr, "Error: --wc requires three arguments\n");
                    return EXIT_FAILURE;
                }
                wc.rgbtRed   = clamp255(atoi(optarg));
                wc.rgbtGreen = clamp255(atoi(argv[optind++]));
                wc.rgbtBlue  = clamp255(atoi(argv[optind++]));
                break;
            case 3:  /* --sc */
                if (optind + 1 >= argc) {
                    fprintf(stderr, "Error: --sc requires three arguments\n");
                    return EXIT_FAILURE;
                }
                sc.rgbtRed   = clamp255(atoi(optarg));
                sc.rgbtGreen = clamp255(atoi(argv[optind++]));
                sc.rgbtBlue  = clamp255(atoi(argv[optind++]));
                break;
            case 4:  /* --ec */
                if (optind + 1 >= argc) {
                    fprintf(stderr, "Error: --ec requires three arguments\n");
                    return EXIT_FAILURE;
                }
                ec.rgbtRed   = clamp255(atoi(optarg));
                ec.rgbtGreen = clamp255(atoi(argv[optind++]));
                ec.rgbtBlue  = clamp255(atoi(argv[optind++]));
                break;
            case 5:
                seed = strtoul(optarg, NULL, 10);
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                usage(argv[0]);
                return EXIT_SUCCESS;
            case 6:
                printf("%s %s\n", argv[0], VERSION);
                return EXIT_SUCCESS;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    /* default filename if none provided */
    if (out_filename[0] == '\0') {
        time_t now = time(NULL);
        snprintf(out_filename, sizeof out_filename,
                 "maze_%zux%zu_%ld.bmp",
                 width, height, (long)now);
    }

    /* validate extension */
    if (!ends_with(out_filename, ".bmp")) {
        fprintf(stderr, "Error: output filename must end in .bmp\n");
        return EXIT_FAILURE;
    }

    /* sanity checks */
    if (width == 0 || height == 0 ||
        cell_size <= 2 || wall_th < 1 || wall_th > cell_size/2 ||
        start.x < 0 || start.x >= (int)width ||
        start.y < 0 || start.y >= (int)height ||
        endp.x  < 0 || endp.x  >= (int)width ||
        endp.y  < 0 || endp.y  >= (int)height)
    {
        fprintf(stderr, "Error: invalid parameters\n");
        return EXIT_FAILURE;
    }

    if (verbose) {
        fprintf(stderr,
            "DEBUG: dims=%zux%zu, cell_size=%d, wall_thick=%d, seed=%lu\n"
            "       start=(%d,%d), end=(%d,%d)\n"
            "       bgc=(%u,%u,%u), wc=(%u,%u,%u), sc=(%u,%u,%u), ec=(%u,%u,%u)\n"
            "       outfile=%s\n",
            width, height, cell_size, wall_th, seed,
            start.x, start.y, endp.x, endp.y,
            bgc.rgbtRed,bgc.rgbtGreen,bgc.rgbtBlue,
            wc.rgbtRed,wc.rgbtGreen,wc.rgbtBlue,
            sc.rgbtRed,sc.rgbtGreen,sc.rgbtBlue,
            ec.rgbtRed,ec.rgbtGreen,ec.rgbtBlue,
            out_filename
        );
    }

    /* start total timer */
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    /* 1) create maze */
    Maze *m;
    {
        struct timespec s, e;
        clock_gettime(CLOCK_MONOTONIC, &s);
        m = maze_create(width, height, cell_size, start, endp, wall_th);
        if (!m) { fprintf(stderr, "Error: maze_create() failed\n"); return EXIT_FAILURE; }
        clock_gettime(CLOCK_MONOTONIC, &e);
        printf("maze_create() completed in %.3f ms\n", diff_ms(&s, &e));
    }

    /* 2) generate DFS maze */
    {
        struct timespec s, e;
        clock_gettime(CLOCK_MONOTONIC, &s);
        maze_generate_dfs(m);
        clock_gettime(CLOCK_MONOTONIC, &e);
        printf("maze_generate_dfs() completed in %.3f ms\n", diff_ms(&s, &e));
    }

    /* 3) create BMP buffer */
    BmpImage *img;
    {
        struct timespec s, e;
        clock_gettime(CLOCK_MONOTONIC, &s);
        img = bmp_create(width*cell_size, height*cell_size);
        if (!img) { fprintf(stderr, "Error: bmp_create() failed\n"); maze_free(m); return EXIT_FAILURE; }
        clock_gettime(CLOCK_MONOTONIC, &e);
        printf("bmp_create() completed in %.3f ms\n", diff_ms(&s, &e));
    }

    /* apply colors */
    MAZE_BG_COLOR    = bgc;
    MAZE_WALL_COLOR  = wc;
    MAZE_START_COLOR = sc;
    MAZE_END_COLOR   = ec;

    /* 4) render maze */
    {
        struct timespec s, e;
        clock_gettime(CLOCK_MONOTONIC, &s);
        maze_render_to_bmp(m, img);
        clock_gettime(CLOCK_MONOTONIC, &e);
        printf("maze_render_to_bmp() completed in %.3f ms\n", diff_ms(&s, &e));
    }

    /* 5) save BMP */
    {
        struct timespec s, e;
        clock_gettime(CLOCK_MONOTONIC, &s);
        if (!bmp_save(out_filename, img)) {
            fprintf(stderr, "Error: bmp_save() failed\n");
            bmp_free(img);
            maze_free(m);
            return EXIT_FAILURE;
        }
        clock_gettime(CLOCK_MONOTONIC, &e);
        printf("bmp_save() completed in %.3f ms\n", diff_ms(&s, &e));
    }

    /* end total timer */
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Total execution time: %.3f ms\n", diff_ms(&t0, &t1));

    /* cleanup */
    bmp_free(img);
    maze_free(m);
    return EXIT_SUCCESS;
}

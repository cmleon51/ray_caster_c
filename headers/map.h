#ifndef MAP_H
#define MAP_H

#include <SDL3/SDL_thread.h>
#include <player.h>
#include <sdl_utils.h>

typedef int WallType;

typedef enum {
    X_SIDE,
    Y_SIDE
} SIDE_HIT;

typedef struct {
    int wall_height;
    int full_wall_height;
    int wall_height_clip;
    double wall_column_hit;
    WallType wall_hit;
    SIDE_HIT side_hit;
    Vec2 wall_start;
    Vec2 wall_end;
} RayHit;

typedef struct {
    int map_width;
    int map_height;
    WallType *map_2d;
    WallType wall_empty;
} Map;

void map_raycast(Map *map, RayHit *rays_arr, Player *player, int start_column, int end_column, int column_max_width, int column_max_height);

WallType map_check_wall(Map *map, int x, int y);

#endif //MAP_H

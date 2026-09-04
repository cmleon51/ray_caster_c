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
    int wall_top;
    int wall_bottom;
    double wall_texture_v;
    double wall_texture_v_step;
    double wall_column_hit;
    WallType wall_hit;
    SIDE_HIT side_hit;

    Vec2 floor_dir;
} RayHit;

typedef struct {
    int width;
    int height;
    WallType *map_2d;
    WallType wall_empty;
} Map;

void map_raycast(Map *map, RayHit *rays_arr, Player *player, int start_column, int end_column, int column_max_width, int column_max_height);

inline WallType map_check_wall(Map *map, int x, int y) {
    return *(map->map_2d + y * map->width + x);
}

#endif //MAP_H

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

typedef struct MapPortion MapPortion;

typedef struct {
    int map_width;
    int map_height;
    int portions_count;
    WallType *map_2d;
    WallType wall_empty;
    MapPortion *portions;
    SDL_AtomicInt *threads_running;
    SDL_Surface *surface;
    SDL_Color (*get_wall_type_color)(WallType wall, SIDE_HIT side_hit, double wall_column_hit, int wall_height, int y_hit);
} Map;

void map_create(Map *map_to_fill, Player *player, int map_width, int map_height, WallType *map_2d, WallType wall_empty,
                SDL_Color (*get_wall_type_color)(WallType wall, SIDE_HIT side_hit, double wall_column_hit, int wall_height, int y_hit));

void map_delete(Map *map);

void map_draw(Map *map, SDL_Surface *surface);

WallType map_check_wall(Map *map, int x, int y);

#endif //MAP_H

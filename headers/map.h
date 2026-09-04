#ifndef MAP_H
#define MAP_H

#include <linear_algebra/vec2.h>
#include <player.h>

typedef int CellType;

typedef struct {
    int width;
    int height;
    CellType *map_2d;
    CellType wall_empty;
} Map;

inline CellType map_check_intersection(Map *map, int x, int y) {
    return *(map->map_2d + y * map->width + x);
}

#endif //MAP_H

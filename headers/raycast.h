#ifndef RAYCAST_H
#define RAYCAST_H

#include <linear_algebra/vec2.h>
#include <map.h>
#include <camera.h>

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
    int wall_hit;
    SIDE_HIT side_hit;

    Vec2 floor_dir;
} RayHit;

void raycast_walls(Map *walls_map, Map *floor_map, Map *ceiling_map, RayHit *rays_arr, Camera *player,
                   int start_column, int end_column, int column_max_width, int column_max_height);


#endif // RAYCAST_H

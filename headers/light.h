#ifndef LIGHT_H
#define LIGHT_H
#include <platform/utils.h>
#include <stdlib.h>
#include <linear_algebra/vec2.h>
#include <linear_algebra/vec3.h>
#include <map.h>

// instead of lowering the performance of the cpu through bilinear filtering I decided to use more RAM for the lightmaps
#define LIGHTMAP_RESOLUTION 64

typedef struct {
    double radius;
    double intensity;
    Vec3 position;
} Light;

typedef enum {
    SURFACE_TOP    = 1,
    SURFACE_BOTTOM = 2,

    SURFACE_RIGHT  = 4,
    SURFACE_LEFT   = 8,

    SURFACE_FRONT  = 16,
    SURFACE_BACK   = 32
} SURFACE_SIDE;

typedef struct {
    SURFACE_SIDE side;
    double lumels[LIGHTMAP_RESOLUTION][LIGHTMAP_RESOLUTION];
} Lumel;

typedef struct {
    int width;
    int height;
    int sides_count;
    Lumel *lumels;
} LightMap;

inline double lightmap_get_lumel_value(LightMap *lightmap, const Vec2 map_cell, const Vec2 norm_lumel, SURFACE_SIDE cell_side) {
    if (norm_lumel.x > 1.0 || norm_lumel.x < 0.0 || norm_lumel.y > 1.0 || norm_lumel.y < 0.0) {
        utils_log(LOG_ERROR, "In function %s the parameter 'norm_lumel' has been called with values outside of the 0.0 to 1.0 range", __FUNCTION__);
        exit(1);
    }

    if (map_cell.x >= lightmap->width || map_cell.y >= lightmap->height || map_cell.x < 0 || map_cell.y < 0)
        return 0.0;

    Lumel *current_lumel = lightmap->lumels + (int)((map_cell.y * lightmap->width + map_cell.x) * lightmap->sides_count);

    for (int i = 0; i < lightmap->sides_count; i++) {
        if (current_lumel[i].side == cell_side) {
            int lumel_x = norm_lumel.x * LIGHTMAP_RESOLUTION;
            int lumel_y = norm_lumel.y * LIGHTMAP_RESOLUTION;

            return current_lumel[i].lumels[lumel_x][lumel_y];
        }
    }

    return 0.0;
}

LightMap *lightmap_create(Light *static_lights, int lights_count, Map *map, Vec3 cell_offset, SURFACE_SIDE cell_sides);

void lightmap_free(LightMap *lightmap);

#endif // LIGHT_H

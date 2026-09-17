#ifndef LIGHT_H
#define LIGHT_H
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

static inline Lumel *lightmap_get_lumel(LightMap *lightmap, int x, int y, SURFACE_SIDE cell_side) {
    if (x >= lightmap->width || y >= lightmap->height || x < 0 || y < 0)
        return NULL;

    Lumel *current_lumel = lightmap->lumels + ((y * lightmap->width + x) * lightmap->sides_count);

    for (int i = 0; i < lightmap->sides_count; i++) {
        if (current_lumel[i].side == cell_side)
            return &current_lumel[i];
    }

    return NULL;
}

LightMap *lightmap_create(Light *static_lights, int lights_count, Map *map, Vec3 cell_offset, SURFACE_SIDE cell_sides);

void lightmap_free(LightMap *lightmap);

#endif // LIGHT_H

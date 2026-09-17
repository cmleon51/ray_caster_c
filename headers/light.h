#ifndef LIGHT_H
#define LIGHT_H
#include <stdlib.h>
#include <linear_algebra/vec2.h>
#include <map.h>

#define LIGHTMAP_RESOLUTION 16

typedef struct {
    double radius;
    double intensity;
    Vec2 light_pos;
} Light;

typedef struct {
    double lumels[LIGHTMAP_RESOLUTION][LIGHTMAP_RESOLUTION];
} Lumel;

typedef struct {
    int width;
    int height;
    Lumel *lumels;
} LightMap;

static inline Lumel *lightmap_get_lumel(LightMap *lightmap, int x, int y) {
    if (x > lightmap->width || y > lightmap->height)
        return NULL;

    return lightmap->lumels + y * lightmap->width + x;
}

LightMap *lightmap_create(Light *static_lights, int lights_count, Map *map,
                          double (*get_cell_distance_from_light)(Light *light, double cell_x, double cell_y));

void lightmap_free(LightMap *lightmap);

#endif // LIGHT_H

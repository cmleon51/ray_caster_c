#ifndef LEVEL_H
#define LEVEL_H

#include <light.h>
#include <linear_algebra/vec2.h>
#include <linear_algebra/vec3.h>
#include <map.h>
#include <sprite.h>
#include <textures.h>

typedef struct {
    int texture;
    double radius;
    double intensity;
} LevelLight;

typedef struct {
    int size;
    Vec2 player_position;
    double player_look_at;
    Map walls_map;
    Map floor_map;
    Map ceiling_map;
    Texture *textures;
    int textures_count;
    Sprite *sprites;
    int sprites_count;
    LevelLight *lights;
    int lights_count;
} Level;

Level *levels_load(int *levels_count);

void levels_free(Level *levels, int levels_count);

void level_get_world_static_lights(Level *level, Map *map, Light **static_lights, int *lights_count, Vec3 light_offset);

#endif // LEVEL_H

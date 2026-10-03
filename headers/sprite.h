#ifndef SPRITE_H
#define SPRITE_H

#include <linear_algebra/math_utils.h>
#include <camera.h>
#include <platform/window.h>
#include <linear_algebra/vec2.h>
#include <textures.h>

typedef struct {
    Vec2 position;
    Texture *texture;
} Sprite;

void sprite_sort(Sprite *sprites_array, int sprites_count, Camera *camera);

void sprite_render(const Sprite *sprite_to_render, const Camera *camera, Window *window, int map_width, int map_height, const double *z_buffer);

#endif // SPRITE_H

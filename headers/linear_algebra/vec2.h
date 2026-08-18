#ifndef VEC2_H
#define VEC2_H

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>

#define MAP_NORMALIZED_COORDINATES(PIXEL, TOP) (PIXEL * TOP)

typedef struct {
    double x;
    double y;
} Vec2;

void vec2_add_double(Vec2 *vec, double addition);

void vec2_subtract_double(Vec2 *vec, double subtraction);

void vec2_subtract_vec2(Vec2 *vec, Vec2 other);

void vec2_add_vec2(Vec2 *vec, Vec2 other);

void vec2_scale(Vec2 *vec, double scale);

void vec2_modify_length(Vec2 *vec, double target_length);

Vec2 vec2_map_to_coord(Vec2 vec_to_modify, int window_width,
                                  int window_height);

Vec2 vec2_from_angle(double angle);

double vec2_get_length(Vec2 *vec);

#endif

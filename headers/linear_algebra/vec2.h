#ifndef VEC2_H
#define VEC2_H

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>

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

void vec2_normalize(Vec2 *vec);

Vec2 vec2_map_norm_coord(Vec2 vec_to_modify, double max_x,
                                  double max_y);

Vec2 vec2_from_angle(double angle);

double vec2_get_length(Vec2 *vec);

#endif

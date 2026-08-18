#include <linear_algebra/vec2.h>

void vec2_add_double(Vec2 *vec, double addition) {
    vec->x += addition;
    vec->y += addition;
}

void vec2_subtract_double(Vec2 *vec, double subtraction) {
    vec->x -= subtraction;
    vec->y -= subtraction;
}

void vec2_subtract_vec2(Vec2 *vec, Vec2 other) {
    vec->x -= other.x;
    vec->y -= other.y;
}

void vec2_add_vec2(Vec2 *vec, Vec2 other) {
    vec->x += other.x;
    vec->y += other.y;
}

void vec2_scale(Vec2 *vec, double scale) {
    vec->x *= scale;
    vec->y *= scale;
}

void vec2_modify_length(Vec2 *vec, double target_length) {
    double current_vec_lengt = vec2_get_length(vec);
    double length_change = target_length / current_vec_lengt;

    vec->x *= length_change;
    vec->y *= length_change;
}

Vec2 vec2_map_to_coord(Vec2 vec_to_modify, int window_width,
                                  int window_height) {
    Vec2 result = {
        .x = MAP_NORMALIZED_COORDINATES(vec_to_modify.x, window_width),
        .y = MAP_NORMALIZED_COORDINATES(vec_to_modify.y, window_height),
    };

    return result;
}

Vec2 vec2_from_angle(double angle) {
    angle *= SDL_PI_F / 180.0;

    double cos_a = SDL_cos(angle);
    double sin_a = SDL_sin(angle);

    return (Vec2){ .x = cos_a, .y = sin_a };
}

double vec2_get_length(Vec2 *vec) {
    return SDL_sqrt((SDL_pow(vec->x, 2.0) + SDL_pow(vec->y, 2.0)));
}

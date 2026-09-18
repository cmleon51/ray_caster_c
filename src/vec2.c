#include <linear_algebra/vec2.h>
#include <linear_algebra/math_utils.h>

#include <math.h>

/**
 *
 * MODIFY PARAMETER
 *
*/

void vec2_add_double(Vec2 *vec, double addition) {
    vec->x += addition;
    vec->y += addition;
}

void vec2_subtract_double(Vec2 *vec, double subtraction) {
    vec->x -= subtraction;
    vec->y -= subtraction;
}

void vec2_subtract_vec2(Vec2 *vec, const Vec2 *other) {
    vec->x -= other->x;
    vec->y -= other->y;
}

void vec2_add_vec2(Vec2 *vec, const Vec2 *other) {
    vec->x += other->x;
    vec->y += other->y;
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

void vec2_map_norm_coord(Vec2 *vec, double max_x, double max_y) {
    vec->x *= max_x;
    vec->y *= max_y;
}

/**
 *
 * RETURN COPY
 *
*/

Vec2 vec2_add_double_cp(const Vec2 *vec, double addition) {
    Vec2 result_vec = *vec;

    result_vec.x += addition;
    result_vec.y += addition;

    return result_vec;
}

Vec2 vec2_subtract_double_cp(const Vec2 *vec, double subtraction) {
    Vec2 result_vec = *vec;

    result_vec.x -= subtraction;
    result_vec.y -= subtraction;

    return result_vec;
}

Vec2 vec2_subtract_vec2_cp(const Vec2 *vec, Vec2 *other) {
    Vec2 result_vec = *vec;

    result_vec.x -= other->x;
    result_vec.y -= other->y;

    return result_vec;
}

Vec2 vec2_add_vec2_cp(const Vec2 *vec, Vec2 *other) {
    Vec2 result_vec = *vec;

    result_vec.x += other->x;
    result_vec.y += other->y;

    return result_vec;
}

Vec2 vec2_scale_cp(const Vec2 *vec, double scale) {
    Vec2 result_vec = *vec;

    result_vec.x *= scale;
    result_vec.y *= scale;

    return result_vec;
}

Vec2 vec2_modify_length_cp(const Vec2 *vec, double target_length) {
    Vec2 result_vec = *vec;
    double current_vec_lengt = vec2_get_length(vec);
    double length_change = target_length / current_vec_lengt;

    result_vec.x *= length_change;
    result_vec.y *= length_change;

    return result_vec;
}

Vec2 vec2_normalize_cp(const Vec2 *vec) {
    Vec2 result_vec = *vec;
    double vec_length = vec2_get_length(vec);

    result_vec.x /= vec_length;
    result_vec.y /= vec_length;

    return result_vec;
}

Vec2 vec2_map_norm_coord_cp(const Vec2 *vec_to_modify, double max_x, double max_y) {
    Vec2 result_vec = {
        .x = vec_to_modify->x * max_x,
        .y = vec_to_modify->y * max_y,
    };

    return result_vec;
}

void vec2_normalize(Vec2 *vec) {
    double vec_length = vec2_get_length(vec);

    vec->x /= vec_length;
    vec->y /= vec_length;
}

/**
*
* UTILS
*
*/

Vec2 vec2_from_angle(double degs) {
    Vec2 result_vec = {};
    degs = DEG_TO_RADS(degs);

    result_vec = (Vec2) {
        .x = cos(degs),
        .y = sin(degs)
    };

    return result_vec;
}

double vec2_get_length(const Vec2 *vec) {
    return sqrt(vec->x * vec->x + vec->y * vec->y);
}

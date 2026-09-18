#ifndef VEC2_H
#define VEC2_H

typedef struct {
    double x;
    double y;
} Vec2;

/**
 *
 * MODIFY PARAMETER
 *
*/

void vec2_add_double(Vec2 *vec, double addition);

void vec2_subtract_double(Vec2 *vec, double subtraction);

void vec2_subtract_vec2(Vec2 *vec, const Vec2 *other);

void vec2_add_vec2(Vec2 *vec, const Vec2 *other);

void vec2_scale(Vec2 *vec, double scale);

void vec2_modify_length(Vec2 *vec, double target_length);

void vec2_normalize(Vec2 *vec);

void vec2_map_norm_coord(Vec2 *vec, double max_x, double max_y);

/**
 *
 * RETURN COPY
 *
*/

Vec2 vec2_add_double_cp(const Vec2 *vec, double addition);

Vec2 vec2_subtract_double_cp(const Vec2 *vec, double subtraction);

Vec2 vec2_subtract_vec2_cp(const Vec2 *vec, Vec2 *other);

Vec2 vec2_add_vec2_cp(const Vec2 *vec, Vec2 *other);

Vec2 vec2_scale_cp(const Vec2 *vec, double scale);

Vec2 vec2_modify_length_cp(const Vec2 *vec, double target_length);

Vec2 vec2_normalize_cp(const Vec2 *vec);

Vec2 vec2_map_norm_coord_cp(const Vec2 *vec_to_modify, double max_x, double max_y);

/**
*
* UTILS
*
*/

Vec2 vec2_from_angle(double degs);

double vec2_get_length(const Vec2 *vec);

#endif

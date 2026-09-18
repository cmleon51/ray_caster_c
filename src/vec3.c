#include <linear_algebra/vec3.h>
#include <linear_algebra/math_utils.h>
#include <math.h>

/**
 *
 * MODIFY PARAMETER
 *
*/

void vec3_add_double(Vec3 *vec, double addition) {
    vec->x += addition;
    vec->y += addition;
    vec->z += addition;
}

void vec3_subtract_double(Vec3 *vec, double subtraction) {
    vec->x -= subtraction;
    vec->y -= subtraction;
    vec->z -= subtraction;
}

void vec3_subtract_vec3(Vec3 *vec, Vec3 other) {
    vec->x -= other.x;
    vec->y -= other.y;
    vec->z -= other.z;
}

void vec3_add_vec3(Vec3 *vec, Vec3 other) {
    vec->x += other.x;
    vec->y += other.y;
    vec->z += other.z;
}

void vec3_scale(Vec3 *vec, double scale) {
    vec->x *= scale;
    vec->y *= scale;
    vec->z *= scale;
}

void vec3_modify_length(Vec3 *vec, double target_length) {
    double current_vec_lengt = vec3_get_length(vec);
    double length_change = target_length / current_vec_lengt;

    vec->x *= length_change;
    vec->y *= length_change;
    vec->z *= length_change;
}

void vec3_map_norm_coord(Vec3 *vec_to_modify, double max_x, double max_y, double max_z) {
    vec_to_modify->x *= max_x;
    vec_to_modify->y *= max_y;
    vec_to_modify->z *= max_z;
}

/**
 *
 * RETURN COPY
 *
*/

Vec3 vec3_normalize_cp(const Vec3 *vec) {
    Vec3 result_vec = *vec;
    double vec_length = vec3_get_length(&result_vec);

    result_vec.x /= vec_length;
    result_vec.y /= vec_length;
    result_vec.z /= vec_length;

    return result_vec;
}

Vec3 vec3_add_double_cp(const Vec3 *vec, double addition) {
    Vec3 result_vec = *vec;

    result_vec.x += addition;
    result_vec.y += addition;
    result_vec.z += addition;

    return result_vec;
}

Vec3 vec3_subtract_double_cp(const Vec3 *vec, double subtraction) {
    Vec3 result_vec = *vec;

    result_vec.x -= subtraction;
    result_vec.y -= subtraction;
    result_vec.z -= subtraction;

    return result_vec;
}

Vec3 vec3_subtract_vec3_cp(const Vec3 *vec, Vec3 *other) {
    Vec3 result_vec = *vec;

    result_vec.x -= other->x;
    result_vec.y -= other->y;
    result_vec.z -= other->z;

    return result_vec;
}

Vec3 vec3_add_vec3_cp(const Vec3 *vec, Vec3 *other) {
    Vec3 result_vec = *vec;

    result_vec.x += other->x;
    result_vec.y += other->y;
    result_vec.z += other->z;

    return result_vec;
}

Vec3 vec3_scale_cp(const Vec3 *vec, double scale) {
    Vec3 result_vec = *vec;

    result_vec.x *= scale;
    result_vec.y *= scale;
    result_vec.z *= scale;

    return result_vec;
}

Vec3 vec3_modify_length_cp(const Vec3 *vec, double target_length) {
    Vec3 result_vec = *vec;
    double current_vec_lengt = vec3_get_length(&result_vec);
    double length_change = target_length / current_vec_lengt;

    result_vec.x *= length_change;
    result_vec.y *= length_change;
    result_vec.z *= length_change;

    return result_vec;
}

/**
*
* UTILS
*
*/

double vec3_get_length(Vec3 *vec) {
    return sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
}

#ifndef VEC3_H
#define VEC3_H

typedef struct {
    double x;
    double y;
    double z;
} Vec3;

/**
 *
 * MODIFY PARAMETER
 *
*/

void vec3_add_double(Vec3 *vec, double addition);

void vec3_subtract_double(Vec3 *vec, double subtraction);

void vec3_subtract_vec3(Vec3 *vec, const Vec3 *other);

void vec3_add_vec3(Vec3 *vec, const Vec3 *other);

void vec3_scale(Vec3 *vec, double scale);

void vec3_modify_length(Vec3 *vec, double target_length);

void vec3_normalize(Vec3 *vec);

void vec3_map_norm_coord(Vec3 *vec_to_modify, double max_x, double max_y, double max_z);

/**
 *
 * RETURN COPY
 *
*/

Vec3 vec3_add_double_cp(const Vec3 *vec, double addition);

Vec3 vec3_subtract_double_cp(const Vec3 *vec, double subtraction);

Vec3 vec3_subtract_vec3_cp(const Vec3 *vec, Vec3 *other);

Vec3 vec3_add_vec3_cp(const Vec3 *vec, Vec3 *other);

Vec3 vec3_scale_cp(const Vec3 *vec, double scale);

Vec3 vec3_modify_length_cp(const Vec3 *vec, double target_length);

Vec3 vec3_normalize_cp(const Vec3 *vec);

Vec3 vec3_map_norm_coord_cp(const Vec3 *vec_to_modify, double max_x, 
                         double max_y, double max_z);

/**
*
* UTILS
*
*/

double vec3_get_length(Vec3 *vec);

#endif

#include <map.h>
#include <light.h>
#include <math.h>
#include <stdlib.h>

double light_get_intensity(double distance, double intensity, double radius) {
    if (distance >= radius)
        return 0.0;

    double distance_ratio = distance / radius;
    double window = 1.0 - pow(distance_ratio, 4.0);

    return intensity * (window * window) / (distance * distance + 1.0);
}

double get_cell_distance_from_light(Light *light, Vec3 lumel_pos) {
    vec3_subtract_vec3(&lumel_pos, &light->position);
    return vec3_get_length(&lumel_pos);
}

LightMap *lightmap_create(Light *static_lights, int lights_count, Map *map, Vec3 cell_offset, SURFACE_SIDE cell_sides) {
    LightMap *light_map = malloc(sizeof(LightMap));
    int sides_count = 0;

    if (cell_sides & SURFACE_FRONT)  sides_count++;
    if (cell_sides & SURFACE_BACK)   sides_count++;
    if (cell_sides & SURFACE_RIGHT)  sides_count++;
    if (cell_sides & SURFACE_LEFT)   sides_count++;
    if (cell_sides & SURFACE_TOP)    sides_count++;
    if (cell_sides & SURFACE_BOTTOM) sides_count++;

    light_map->width = map->width;
    light_map->height = map->height;
    light_map->sides_count = sides_count;
    light_map->lumels = malloc(sizeof(Lumel) * map->width * map->height * sides_count);

    double cell_lumel_ratio = 1.0 / LIGHTMAP_RESOLUTION;
    Lumel *current_lumel = light_map->lumels;

    /**
     *
     * CALCULATE THE LUMEL FOR EVERY CELL AND EVERY "SIDE" OF THE CELL
     *
    */
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            Vec3 cell_center = {
                .x = x + 0.5,
                .y = y + 0.5,
                .z = 0.0
            };
            vec3_add_vec3(&cell_center, &cell_offset);

            Vec3 surface_front_pos = {
                .x = cell_center.x - 0.5,
                .y = cell_center.y + 0.5,
                .z = cell_center.z + 0.5
            };
            Vec3 surface_back_pos = {
                .x = cell_center.x - 0.5,
                .y = cell_center.y - 0.5,
                .z = cell_center.z + 0.5
            };

            Vec3 surface_right_pos = {
                .x = cell_center.x + 0.5,
                .y = cell_center.y - 0.5,
                .z = cell_center.z + 0.5
            };
            Vec3 surface_left_pos = {
                .x = cell_center.x - 0.5,
                .y = cell_center.y - 0.5,
                .z = cell_center.z + 0.5
            };

            Vec3 surface_up_pos = {
                .x = cell_center.x - 0.5,
                .y = cell_center.y - 0.5,
                .z = cell_center.z + 0.5
            };
            Vec3 surface_down_pos = {
                .x = cell_center.x - 0.5,
                .y = cell_center.y - 0.5,
                .z = cell_center.z - 0.5
            };

            for (int i = 0; i < LIGHTMAP_RESOLUTION; i++) {
                for (int j = 0; j < LIGHTMAP_RESOLUTION; j++) {
                    double lumel_x = (i + 0.5) * cell_lumel_ratio;
                    double lumel_y = (j + 0.5) * cell_lumel_ratio;

                    for (int side = 0; side < sides_count; side++) {
                        (current_lumel + side)->lumels[i][j] = 0.0;
                    }

                    // calculate the current lumel position based upon the starting surface positions
                    Vec3 surface_front_lumel = {
                        .x = surface_front_pos.x + lumel_x,
                        .y = surface_front_pos.y,
                        .z = surface_front_pos.z - lumel_y
                    };
                    Vec3 surface_back_lumel = {
                        .x = surface_back_pos.x + lumel_x,
                        .y = surface_back_pos.y,
                        .z = surface_back_pos.z - lumel_y
                    };

                    Vec3 surface_right_lumel = {
                        .x = surface_right_pos.x,
                        .y = surface_right_pos.y + lumel_x,
                        .z = surface_right_pos.z - lumel_y
                    };
                    Vec3 surface_left_lumel = {
                        .x = surface_left_pos.x,
                        .y = surface_left_pos.y + lumel_x,
                        .z = surface_left_pos.z - lumel_y
                    };

                    Vec3 surface_up_lumel = {
                        .x = surface_up_pos.x + lumel_x,
                        .y = surface_up_pos.y + lumel_y,
                        .z = surface_up_pos.z
                    };
                    Vec3 surface_down_lumel = {
                        .x = surface_down_pos.x + lumel_x,
                        .y = surface_down_pos.y + lumel_y,
                        .z = surface_down_pos.z
                    };

                    for (int light = 0; light < lights_count; light++) {
                        Light *current_light = &static_lights[light];

                        if (cell_sides & SURFACE_FRONT) {
                            double front_distance = get_cell_distance_from_light(current_light, surface_front_lumel);
                            current_lumel->side = SURFACE_FRONT;
                            current_lumel->lumels[i][j] += light_get_intensity(front_distance, current_light->intensity, current_light->radius);
                            current_lumel++;
                        }

                        if (cell_sides & SURFACE_BACK) {
                            double back_distance = get_cell_distance_from_light(current_light, surface_back_lumel);
                            current_lumel->side = SURFACE_BACK;
                            current_lumel->lumels[i][j] += light_get_intensity(back_distance, current_light->intensity, current_light->radius);
                            current_lumel++;
                        }

                        if (cell_sides & SURFACE_RIGHT) {
                            double right_distance = get_cell_distance_from_light(current_light, surface_right_lumel);
                            current_lumel->side = SURFACE_RIGHT;
                            current_lumel->lumels[i][j] += light_get_intensity(right_distance, current_light->intensity, current_light->radius);
                            current_lumel++;
                        }

                        if (cell_sides & SURFACE_LEFT) {
                            double left_distance = get_cell_distance_from_light(current_light, surface_left_lumel);
                            current_lumel->side = SURFACE_LEFT;
                            current_lumel->lumels[i][j] += light_get_intensity(left_distance, current_light->intensity, current_light->radius);
                            current_lumel++;
                        }

                        if (cell_sides & SURFACE_TOP) {
                            double up_distance = get_cell_distance_from_light(current_light, surface_up_lumel);
                            current_lumel->side = SURFACE_TOP;
                            current_lumel->lumels[i][j] += light_get_intensity(up_distance, current_light->intensity, current_light->radius);
                            current_lumel++;
                        }

                        if (cell_sides & SURFACE_BOTTOM) {
                            double down_distance = get_cell_distance_from_light(current_light, surface_down_lumel);
                            current_lumel->side = SURFACE_BOTTOM;
                            current_lumel->lumels[i][j] += light_get_intensity(down_distance, current_light->intensity, current_light->radius);
                            current_lumel++;
                        }

                        current_lumel -= sides_count;
                    }
                }
            }

            current_lumel += sides_count;
        }
    }

    return light_map;
}

void lightmap_free(LightMap *lightmap) {
    free(lightmap);
    free(lightmap->lumels);
}

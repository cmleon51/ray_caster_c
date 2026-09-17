#include <map.h>
#include <light.h>
#include <math.h>
#include <stdlib.h>

double _light_get_intensity(double distance, double intensity, double radius) {
    if (distance >= radius)
        return 0.0;

    double distance_ratio = distance / radius;
    double window = 1.0 - pow(distance_ratio, 4.0);

    return intensity * (window * window) / (distance * distance + 1.0);
}

LightMap *lightmap_create(Light *static_lights, int lights_count, Map *map,
                          double (*get_cell_distance_from_light)(Light *light, double cell_x, double cell_y)) {
    LightMap *light_map = malloc(sizeof(LightMap));
    light_map->width = map->width;
    light_map->height = map->height;
    light_map->lumels = malloc(sizeof(Lumel) * map->width * map->height);

    double cell_lumel_ratio = 1.0 / LIGHTMAP_RESOLUTION;

    for (int x = 0; x < map->width; x++) {
        for (int y = 0; y < map->height; y++) {
            // go over every lumel to calculate the light intensity
            for (int i = 0; i < LIGHTMAP_RESOLUTION; i++) {
                for (int j = 0; j < LIGHTMAP_RESOLUTION; j++) {
                    Lumel *current_lumel = lightmap_get_lumel(light_map, x, y);
                    double cell_x = (i + 0.5) * cell_lumel_ratio + x;
                    double cell_y = (j + 0.5) * cell_lumel_ratio + y;

                    current_lumel->lumels[i][j] = 0.0;

                    for (int light = 0; light < lights_count; light++) {
                        Light *current_light = &static_lights[light];

                        double cell_distance = get_cell_distance_from_light(current_light, cell_x, cell_y);

                        current_lumel->lumels[i][j] += _light_get_intensity(cell_distance, current_light->intensity, current_light->radius);
                    }
                }
            }
        }
    }

    return light_map;
}

void lightmap_free(LightMap *lightmap) {
    free(lightmap->lumels);
}

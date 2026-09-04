#include <assert.h>
#include <raycast.h>

void raycast_walls(Map *walls_map, Map *floor_map, Map *ceiling_map, RayHit *rays_arr, Camera *player,
                   int start_column, int end_column, int column_max_width, int column_max_height) {
    assert(walls_map->width == floor_map->width && ceiling_map->width == floor_map->width);
    assert(walls_map->height == floor_map->height && ceiling_map->height == floor_map->height);

    double player_look_at_rads = DEG_TO_RADS(player->look_at);

    double current_angle = player->look_at - (player->fov / 2.0);
    double display_to_fov_ratio = player->fov / column_max_width;

    current_angle += (display_to_fov_ratio * start_column);

    for (int x = start_column; x < end_column; x++) {
        Vec2 ray_start = vec2_map_norm_coord(player->position, walls_map->width, walls_map->height);
        Vec2 ray_dir = vec2_from_angle(current_angle);

        vec2_normalize(&ray_dir);

        Vec2 ray_unit_step_size = {
            .x = SDL_sqrt(1 + (ray_dir.y / ray_dir.x) * (ray_dir.y / ray_dir.x)),
            .y = SDL_sqrt(1 + (ray_dir.x / ray_dir.y) * (ray_dir.x / ray_dir.y)),
        };

        Vec2 map_check = {
            .x = (int)ray_start.x,
            .y = (int)ray_start.y
        };
        Vec2 ray_length_1d;

        Vec2 step = {};

        if (ray_dir.x < 0.0) {
            step.x -= 1;
            ray_length_1d.x = (ray_start.x - map_check.x) * ray_unit_step_size.x;
        } else {
            step.x += 1;
            ray_length_1d.x = ((map_check.x + 1) - ray_start.x) * ray_unit_step_size.x;
        }

        if (ray_dir.y < 0.0) {
            step.y -= 1;
            ray_length_1d.y = (ray_start.y - map_check.y) * ray_unit_step_size.y;
        } else {
            step.y += 1;
            ray_length_1d.y = ((map_check.y + 1) - ray_start.y) * ray_unit_step_size.y;
        }

        double distance_x = 0.0, distance_y = 0.0;
        int side_hit = 0;
        int wall_hit = walls_map->wall_empty;

        while (wall_hit == walls_map->wall_empty && (distance_x < walls_map->width || distance_y < walls_map->height)) {
            if (ray_length_1d.x < ray_length_1d.y) {
                map_check.x += step.x;
                distance_x = ray_length_1d.x;
                ray_length_1d.x += ray_unit_step_size.x;
                side_hit = X_SIDE;
            } else {
                map_check.y += step.y;
                distance_y = ray_length_1d.y;
                ray_length_1d.y += ray_unit_step_size.y;
                side_hit = Y_SIDE;
            }

            wall_hit = map_check_intersection(walls_map, (int)map_check.x, (int)map_check.y);
        }

        if (wall_hit != walls_map->wall_empty) {
            double distance = side_hit == Y_SIDE ? distance_y : distance_x;

            double perspective_correction = SDL_cos(DEG_TO_RADS(current_angle) - player_look_at_rads);
            double correct_perspective_distance = distance * perspective_correction;
            int full_wall_height = (int)(column_max_height / correct_perspective_distance);
            int wall_height = full_wall_height > column_max_height ? column_max_height : full_wall_height;
            int wall_height_clip = (full_wall_height - wall_height) / 2;

            int wall_top = column_max_height / 2 - wall_height / 2;
            int wall_bottom = column_max_height / 2 + wall_height / 2;

            double wall_column_hit = 0.0;

            if (side_hit == X_SIDE)
                wall_column_hit = ray_start.y + distance * ray_dir.y;
            else
                wall_column_hit = ray_start.x + distance * ray_dir.x;

            wall_column_hit -= SDL_floor(wall_column_hit);

            rays_arr[x] = (RayHit) {
                .wall_height = wall_height,
                .wall_top = wall_top,
                .wall_bottom = wall_bottom,
                .wall_texture_v = (double)wall_height_clip / full_wall_height,
                .wall_texture_v_step = 1.0 / full_wall_height,
                .wall_column_hit = wall_column_hit,
                .wall_hit = wall_hit,
                .side_hit = side_hit,
                .floor_dir = {
                    .x = ray_dir.x / perspective_correction,
                    .y = ray_dir.y / perspective_correction,
                },
            };
        }

        current_angle += display_to_fov_ratio;
    }
}

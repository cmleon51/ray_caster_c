#include "vec2.h"
#include <map.h>

#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <player.h>
#include <sdl_utils.h>

WallType map_check_wall(Map *map, int x, int y) {
    return *(map->map_2d + y * map->map_width + x);
}

void map_raycast(Map *map, RayHit *rays_arr, Player *player, int start_column, int end_column, int column_max_width, int column_max_height) {
    double player_look_at_rads = DEG_TO_RADS(player->look_at);

    double current_angle = player->look_at - (player->fov / 2.0);
    double display_to_fov_ratio = player->fov / column_max_width;

    current_angle += (display_to_fov_ratio * start_column);

    for (int x = start_column; x < end_column; x++) {
        Vec2 ray_start = vec2_map_norm_coord(player->position, map->map_width, map->map_height);
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
        WallType wall_hit = map->wall_empty;

        while (wall_hit == map->wall_empty && (distance_x < map->map_width || distance_y < map->map_height)) {
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

            wall_hit = map_check_wall(map, (int)map_check.x, (int)map_check.y);
        }

        if (wall_hit != map->wall_empty) {
            double distance = side_hit == Y_SIDE ? distance_y : distance_x;

            double correct_perspective_distance = distance * SDL_cos(DEG_TO_RADS(current_angle) - player_look_at_rads);
            int full_wall_height = (int)(column_max_height / correct_perspective_distance);
            int wall_height = full_wall_height > column_max_height ? column_max_height : full_wall_height;
            int wall_height_clip = (full_wall_height - wall_height) / 2;

            double half_wall_length = ((double)wall_height / column_max_height) / 2.0;

            double wall_column_hit = 0.0;

            if (side_hit == X_SIDE)
                wall_column_hit = ray_start.y + distance * ray_dir.y;
            else
                wall_column_hit = ray_start.x + distance * ray_dir.x;

            wall_column_hit -= SDL_floor(wall_column_hit);

            Vec2 wall_start = {
                .x = (double)x / column_max_width,
                .y = 0.5 - half_wall_length,
            };
            Vec2 wall_end = {
                .x = (double)x / column_max_width,
                .y = 0.5 + half_wall_length,
            };

            rays_arr[x] = (RayHit) {
                .wall_height = wall_height,
                .full_wall_height = full_wall_height,
                .wall_height_clip = wall_height_clip,
                .wall_column_hit = wall_column_hit,
                .wall_hit = wall_hit,
                .side_hit = side_hit,
                .wall_start = wall_start,
                .wall_end = wall_end,
                .ray_angle = current_angle
            };
        }

        current_angle += display_to_fov_ratio;
    }
}

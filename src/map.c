#include <map.h>

#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <player.h>
#include <sdl_utils.h>

struct MapPortion {
    Player *player;
    int thread_nr;
    SDL_Thread *thread;
    Map *map;
    SDL_Semaphore *starting_semaphore;
    SDL_Semaphore *finished_semaphore;
};

int map_draw_portion(void *args) {
    MapPortion *map_portion = (MapPortion*)args;

    Player *player = map_portion->player;
    int thread_nr = map_portion->thread_nr;

    while (SDL_GetAtomicInt(map_portion->map->threads_running)) {
        SDL_WaitSemaphore(map_portion->starting_semaphore);

        SDL_Surface *surface = map_portion->map->surface;
        int map_portion_step = surface->w / map_portion->map->portions_count;

        int start_x = map_portion_step * thread_nr;
        int end_x = start_x + map_portion_step;

        if (thread_nr == map_portion->map->portions_count - 1 && end_x != surface->w) {
            end_x = surface->w;
        }

        double player_look_at_rads = DEG_TO_RADS(player->look_at);

        double current_angle = player->look_at - (player->fov / 2.0);
        double display_to_fov_ratio = player->fov / surface->w;

        current_angle += (display_to_fov_ratio * start_x);

        for (int x = start_x; x < end_x; x++) {
            Vec2 ray_start = vec2_map_norm_coord(player->position, map_portion->map->map_width, map_portion->map->map_height);
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
            WallType wall_hit = map_portion->map->wall_empty;

            while (wall_hit == map_portion->map->wall_empty && (distance_x < map_portion->map->map_width || distance_y < map_portion->map->map_height)) {
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

                wall_hit = map_check_wall(map_portion->map, (int)map_check.x, (int)map_check.y);
            }

            if (wall_hit != map_portion->map->wall_empty) {
                double distance = side_hit == Y_SIDE ? distance_y : distance_x;

                distance *= SDL_cos(DEG_TO_RADS(current_angle) - player_look_at_rads);
                double half_wall_length = ((surface->h / distance) / surface->h) / 2.0;
                RGBA wall_color;

                half_wall_length = half_wall_length > 0.5 ? 0.5 : half_wall_length;

                wall_color = map_portion->map->get_wall_type_color(wall_hit, side_hit);

                Vec2 wall_start = {
                    .x = (double)x / surface->w,
                    .y = 0.5 - half_wall_length,
                };
                Vec2 wall_end = {
                    .x = (double)x / surface->w,
                    .y = 0.5 + half_wall_length,
                };

                SDLUtils_normalized_FillSurfaceLine(surface, wall_start, wall_end, wall_color);
            }

            current_angle += display_to_fov_ratio;
        }

        SDL_SignalSemaphore(map_portion->finished_semaphore);
    }

    return 0;
}

void map_create(Map *map_to_fill, Player *player, int map_width, int map_height, WallType *map_2d, WallType wall_empty,
                RGBA (*get_wall_type_color)(WallType wall, SIDE_HIT side_hit)) {
    int max_threads = SDL_GetNumLogicalCPUCores();

    MapPortion *portions = (MapPortion *)malloc(sizeof(MapPortion) * max_threads);

    SDL_AtomicInt *threads_running = (SDL_AtomicInt *)malloc(sizeof(SDL_AtomicInt));
    SDL_SetAtomicInt(threads_running, 1);

    map_to_fill->map_width = map_width;
    map_to_fill->map_height = map_height;
    map_to_fill->map_2d = map_2d;
    map_to_fill->portions_count = max_threads;
    map_to_fill->portions = portions;
    map_to_fill->get_wall_type_color = get_wall_type_color;
    map_to_fill->wall_empty = wall_empty;
    map_to_fill->threads_running = threads_running;

    for (int i = 0; i < max_threads; i++) {
        SDL_Semaphore *semaphore = SDL_CreateSemaphore(0);
        SDL_Semaphore *finished_semaphore = SDL_CreateSemaphore(0);

        portions[i] = (MapPortion) {
            .player = player,
            .map = map_to_fill,
            .starting_semaphore = semaphore,
            .thread_nr = i,
            .finished_semaphore = finished_semaphore,
        };

        portions[i].thread = SDL_CreateThread(map_draw_portion, "thread", &portions[i]);
    }
}

void map_delete(Map *map) {
    SDL_SetAtomicInt(map->threads_running, 0);

    for (int i = 0; i < map->portions_count; i++) {
        SDL_Semaphore *semaphore = map->portions[i].starting_semaphore;
        SDL_SignalSemaphore(semaphore);

        SDL_WaitThread(map->portions[i].thread, NULL);

        SDL_DestroySemaphore(semaphore);
        SDL_DestroySemaphore(map->portions[i].finished_semaphore);
    }

    free(map->threads_running);
    free(map->portions);
}

void map_draw(Map *map, SDL_Surface *surface) {
    map->surface = surface;

    for (int i = 0; i < map->portions_count; i++) {
        SDL_Semaphore *semaphore = map->portions[i].starting_semaphore;
        SDL_SignalSemaphore(semaphore);
    }

    for (int i = 0; i < map->portions_count; i++) {
        SDL_WaitSemaphore(map->portions[i].finished_semaphore);
    }
}

WallType map_check_wall(Map *map, int x, int y) {
    return *(map->map_2d + y * map->map_width + x);
}

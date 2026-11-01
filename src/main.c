#include <linear_algebra/vec3.h>
#include <platform/utils.h>
#include <math.h>
#include <stdlib.h>
#include <linear_algebra/vec2.h>
#include <linear_algebra/math_utils.h>
#include <player.h>
#include <camera.h>
#include <textures.h>
#include <raycast.h>
#include <platform/window.h>
#include <platform/threading.h>
#include <light.h>
#include <map.h>
#include <sprite.h>
#include <level.h>

// The player's speed and reach are in map cells, since the camera works in 0.0 to 1.0
// coordinates they get divided by the size of the current level
#define PLAYER_CELLS_PER_SECOND 10.0
#define PLAYER_WALL_COLLISION_CELLS 0.5

typedef struct {
    int thread_nr;
    int max_threads;
    Level *level;
    Player *player;
    RayHit **rays_arr;
    double **z_buffer;
    Map *walls_map;
    Map *floor_map;
    Map *ceiling_map;
    LightMap *floor_light_map;
    LightMap *ceiling_light_map;
    LightMap *walls_light_map;
    Window *game_window;
    AtomicInt running;
    Semaphore start;
    Semaphore finished;
} ThreadData;

typedef struct {
    Light *static_lights;
    int static_lights_count;
    Map *walls_map;
    Map *floor_map;
    Map *ceiling_map;
    LightMap *walls_light_map;
    LightMap *floor_light_map;
    LightMap *ceiling_light_map;
    AtomicInt cancel;
    AtomicInt finished;
} LightmapLoad;

Vec2 window_norm_point(Window *window, int x, int y) {
    return (Vec2) {
        .x = (double)x / window_get_width(window),
        .y = (double)y / window_get_height(window),
    };
}

RGBA map_sample_cell(Texture *textures, Map *map, int cell_x, int cell_y, double tile_x, double tile_y) {
    RGBA empty = {0x00, 0x00, 0x00, 0xFF};

    if (cell_x < 0 || cell_x >= map->width || cell_y < 0 || cell_y >= map->height)
        return empty;

    CellType cell = map_check_intersection(map, cell_x, cell_y);

    if (cell == map->wall_empty)
        return empty;

    Texture *texture = &textures[cell - 1];

    return texture_get_pixel(texture, tile_x * texture->width, tile_y * texture->height);
}

int render_portion(void *args) {
    ThreadData *data = (ThreadData *)args;

    while (thread_get_atomic_int(data->running)) {
        thread_wait_semaphore(data->start);

        if (!thread_get_atomic_int(data->running))
            break;

        Window *window = data->game_window;
        int window_width = window_get_width(window);
        int window_height = window_get_height(window);

        int width_threads_ratio = window_width / data->max_threads;
        int column_start = width_threads_ratio * data->thread_nr;
        int column_end = width_threads_ratio + column_start;

        if (data->thread_nr == data->max_threads - 1)
            column_end = window_width;

        raycast_walls(data->walls_map, data->floor_map, data->ceiling_map, *data->rays_arr,
                      &data->player->camera, column_start, column_end, window_width, window_height);

        for (int x = column_start; x < column_end; x++) {
            RayHit *curr_ray = &(*data->rays_arr)[x];

            /*
             *
             * CEILING AND FLOOR CASTING
             *
             * */

            double horizon = (double)window_height / 2;

            int floor_start = curr_ray->wall_bottom > horizon ? curr_ray->wall_bottom : horizon + 1;
            int span_count = window_height - floor_start;

            if (span_count > 0) {
                RGBA floor_colors[span_count];
                RGBA ceiling_colors[span_count];

                Vec2 player_pos = vec2_map_norm_coord_cp(&data->player->camera.position, data->walls_map->width, data->walls_map->height);

                for (int y = floor_start; y < window_height; y++) {
                    double row_distance = horizon / (y - horizon);

                    double floor_x = player_pos.x + row_distance * curr_ray->floor_dir.x;
                    double floor_y = player_pos.y + row_distance * curr_ray->floor_dir.y;

                    double tile_x = floor_x - floor(floor_x);
                    double tile_y = floor_y - floor(floor_y);

                    int cell_x = floor(floor_x);
                    int cell_y = floor(floor_y);

                    int floor_index = y - floor_start;

                    int ceiling_index = window_height - 1 - y;

                    /**
                     *
                     * CALCULATE LIGHT INTENSITY THROUGH THE LIGHTMAP
                     *
                    */
                    Vec2 lightmap_cell = (Vec2) { .x = cell_x, .y = cell_y };
                    Vec2 lumel_cell = (Vec2) { .x = tile_x, .y = tile_y };

                    double floor_light_intensity = light_get_intensity(&data->player->light, (Vec3) { .x = floor_x, .y = floor_y, .z = -0.5}) + lightmap_get_lumel_value(data->floor_light_map, lightmap_cell, lumel_cell, SURFACE_TOP);
                    double ceiling_light_intensity = light_get_intensity(&data->player->light, (Vec3) { .x = floor_x, .y = floor_y, .z = 0.5}) + lightmap_get_lumel_value(data->ceiling_light_map, lightmap_cell, lumel_cell, SURFACE_BOTTOM);

                    floor_light_intensity = floor_light_intensity > 1.0 ? 1.0 : floor_light_intensity;
                    ceiling_light_intensity = ceiling_light_intensity > 1.0 ? 1.0 : ceiling_light_intensity;

                    /**
                     *
                     * APPLY FLOOR AND CEILING COLORS
                     *
                     */
                    floor_colors[floor_index] =
                        map_sample_cell(data->level->textures, data->floor_map, cell_x, cell_y, tile_x, tile_y);

                    floor_colors[floor_index].r *= floor_light_intensity;
                    floor_colors[floor_index].g *= floor_light_intensity;
                    floor_colors[floor_index].b *= floor_light_intensity;

                    ceiling_colors[ceiling_index] =
                        map_sample_cell(data->level->textures, data->ceiling_map, cell_x, cell_y, tile_x, tile_y);

                    ceiling_colors[ceiling_index].r *= ceiling_light_intensity;
                    ceiling_colors[ceiling_index].g *= ceiling_light_intensity;
                    ceiling_colors[ceiling_index].b *= ceiling_light_intensity;
                }

                window_draw_line(window,
                    window_norm_point(window, x, 0),
                    window_norm_point(window, x, span_count - 1),
                    ceiling_colors, span_count);

                window_draw_line(window,
                    window_norm_point(window, x, floor_start),
                    window_norm_point(window, x, window_height - 1),
                    floor_colors, span_count);
            }

            /*
             *
             * WALL DRAWING
             *
            */

            if (curr_ray->wall_height > 0) {
                RGBA wall_colors[curr_ray->wall_height];
                Texture *current_texture = &data->level->textures[curr_ray->wall_hit - 1];

                int texture_x = curr_ray->wall_column_hit * current_texture->width;
                double texture_v = curr_ray->wall_texture_v;

                Vec3 wall_world_position = {
                    .x = curr_ray->wall_hit_position.x,
                    .y = curr_ray->wall_hit_position.y,
                    .z = 0.0
                };

                for (int y = 0; y < curr_ray->wall_height; y++) {
                    double player_light_intensity = light_get_intensity(&data->player->light, wall_world_position);

                    SURFACE_SIDE wall_side_hit = curr_ray->side_hit == Y_SIDE ?
                        curr_ray->floor_dir.y > 0.0 ? SURFACE_BACK : SURFACE_FRONT
                        : curr_ray->floor_dir.x > 0.0 ? SURFACE_LEFT : SURFACE_RIGHT;

                    double light_intensity = player_light_intensity + lightmap_get_lumel_value(data->walls_light_map, curr_ray->map_cell, (Vec2) { .x = curr_ray->wall_column_hit, .y = texture_v }, wall_side_hit);

                    light_intensity = light_intensity > 1.0 ? 1.0 : light_intensity;

                    int texture_y = (int)(texture_v * current_texture->height);
                    texture_v += curr_ray->wall_texture_v_step;

                    wall_colors[y] = texture_get_pixel(current_texture, texture_x, texture_y);

                    if (curr_ray->side_hit == Y_SIDE) {
                        wall_colors[y].r = wall_colors[y].r >> 1;
                        wall_colors[y].g = wall_colors[y].g >> 1;
                        wall_colors[y].b = wall_colors[y].b >> 1;
                    }

                    wall_colors[y].r *= light_intensity;
                    wall_colors[y].g *= light_intensity;
                    wall_colors[y].b *= light_intensity;
                }

                (*data->z_buffer)[x] = curr_ray->wall_distance;

                window_draw_line(window,
                    window_norm_point(window, x, curr_ray->wall_top),
                    window_norm_point(window, x, curr_ray->wall_bottom),
                    wall_colors, curr_ray->wall_height);
            }
        }

        thread_signal_semaphore(data->finished);
    }

    return 0;
}

int load_lightmaps(void *args) {
    LightmapLoad *load = (LightmapLoad *)args;

    load->floor_light_map = lightmap_create(load->static_lights, load->static_lights_count, load->floor_map, (Vec3) {.z = -1.0}, SURFACE_TOP, load->cancel);
    load->ceiling_light_map = lightmap_create(load->static_lights, load->static_lights_count, load->ceiling_map, (Vec3) {.z = 1.0 }, SURFACE_BOTTOM, load->cancel);
    load->walls_light_map = lightmap_create(load->static_lights, load->static_lights_count, load->walls_map, (Vec3) {},
                                            SURFACE_RIGHT | SURFACE_LEFT | SURFACE_FRONT | SURFACE_BACK, load->cancel);

    thread_set_atomic_int(load->finished, 1);

    return 0;
}

void lightmap_loader_stop(LightmapLoad *load, Thread *loader) {
    if (*loader) {
        thread_set_atomic_int(load->cancel, 1);
        thread_wait(*loader);
        *loader = NULL;
    }
}

void lightmaps_free(LightmapLoad *load) {
    if (load->floor_light_map)
        lightmap_free(load->floor_light_map);
    if (load->ceiling_light_map)
        lightmap_free(load->ceiling_light_map);
    if (load->walls_light_map)
        lightmap_free(load->walls_light_map);

    load->floor_light_map = NULL;
    load->ceiling_light_map = NULL;
    load->walls_light_map = NULL;
}

int main(void) {
    int result = 0;

    /**
     *
     * THREAD VARIABLES
     *
    */

    int max_threads = thread_get_num_logical_cpu_cores();
    Thread threads[max_threads] = {};
    ThreadData thread_data[max_threads] = {};
    AtomicInt running = NULL;
    Thread lightmap_loader = NULL;

    /**
     *
     * GAME VARIABLES
     *
    */


    int static_lights_count = 0;
    Light *world_static_lights = NULL;
    LightmapLoad lightmap_load = {};
    int lightmaps_loaded = 0;

    Level *levels = NULL;
    int levels_count = 0;
    int current_level = 0;
    int level_start_requested = 1;
    Level *level = NULL;

    double *z_buffer = NULL;
    RayHit *rays = NULL;
    Player player = {};

    Event event = EVENT_NONE;

    NsTime time_start_loop = 0;
    NsTime prev_time_start_loop = 0;
    NsTime time_end_loop = 0;
    double delta_time = 0.0;
    double fps_count = 0.0;

    Window *window = window_create();

    if (!window) {
        utils_log(LOG_ERROR, "Couldn't create the application's window");
        result = 1;
        goto cleanup;
    }

    levels = levels_load(&levels_count);

    if (!levels) {
        result = 1;
        goto cleanup;
    }

    player = (Player){
        .camera = {
            .fov = 90.0,
        },
        .light = {
            .intensity = 0.5,
            .radius = 8.0
        },
        .rotation_speed = 200.0
    };

    rays = malloc(sizeof(RayHit) * window_get_width(window));

    z_buffer = calloc(window_get_width(window), sizeof(double));

    running = thread_create_atomic_int();
    thread_set_atomic_int(running, 1);

    for (int i = 0; i < max_threads; i++) {
        thread_data[i] = (ThreadData) {
            .thread_nr = i,
            .max_threads = max_threads,
            .player = &player,
            .rays_arr = &rays,
            .game_window = window,
            .running = running,
            .start = thread_create_semaphore(0),
            .finished = thread_create_semaphore(0),
            .z_buffer = &z_buffer,
        };

        threads[i] = thread_create(render_portion, "thread", &thread_data[i]);
    }

    lightmap_load.cancel = thread_create_atomic_int();
    lightmap_load.finished = thread_create_atomic_int();

    while (!window_should_quit(window)) {
        fps_count = NS_TO_S(time_end_loop - time_start_loop);
        time_start_loop = utils_get_current_time_ns();

        delta_time = NS_TO_S(time_start_loop - prev_time_start_loop);

        prev_time_start_loop = utils_get_current_time_ns();

        while ((event = window_poll_event(window)) != EVENT_NONE) {
            switch (event) {
            case EVENT_NONE:
                break;
            case EVENT_QUIT_APP:
                    window_close(window);
                break;
            case EVENT_WINDOW_RESIZED:
                if ((result = window_resize(window)) != 0) {
                    goto cleanup;
                }

                int window_width = window_get_width(window);

                z_buffer = realloc(z_buffer, sizeof(double) * window_width);
                rays = realloc(rays, sizeof(RayHit) * window_get_width(window));
                break;
            case EVENT_KEY_LEFT:
                current_level = (current_level + levels_count - 1) % levels_count;
                level_start_requested = 1;
                break;
            case EVENT_KEY_RIGHT:
                current_level = (current_level + 1) % levels_count;
                level_start_requested = 1;
                break;
            }
        }

        /**
         *
         * START THE CURRENT LEVEL
         *
         */

        if (level_start_requested) {
            level_start_requested = 0;

            lightmap_loader_stop(&lightmap_load, &lightmap_loader);
            lightmaps_free(&lightmap_load);

            level = &levels[current_level];

            player.camera.position = level->player_position;
            player.camera.look_at = level->player_look_at;
            player.movement_speed = PLAYER_CELLS_PER_SECOND / level->size;
            player.light.position = (Vec3) { .x = level->player_position.x, .y = level->player_position.y, .z = 0.5 };
            vec3_map_norm_coord(&player.light.position, level->size, level->size, 1);

            free(world_static_lights);
            world_static_lights = NULL;
            static_lights_count = 0;

            level_get_world_static_lights(level, &level->floor_map, &world_static_lights, &static_lights_count, (Vec3) {.z = -0.5});
            level_get_world_static_lights(level, &level->ceiling_map, &world_static_lights, &static_lights_count, (Vec3) {.z = 0.5});
            level_get_world_static_lights(level, &level->walls_map, &world_static_lights, &static_lights_count, (Vec3) {});

            for (int i = 0; i < max_threads; i++) {
                thread_data[i].level = level;
                thread_data[i].walls_map = &level->walls_map;
                thread_data[i].floor_map = &level->floor_map;
                thread_data[i].ceiling_map = &level->ceiling_map;
            }

            lightmap_load.static_lights = world_static_lights;
            lightmap_load.static_lights_count = static_lights_count;
            lightmap_load.walls_map = &level->walls_map;
            lightmap_load.floor_map = &level->floor_map;
            lightmap_load.ceiling_map = &level->ceiling_map;

            thread_set_atomic_int(lightmap_load.cancel, 0);
            thread_set_atomic_int(lightmap_load.finished, 0);
            lightmaps_loaded = 0;

            lightmap_loader = thread_create(load_lightmaps, "lightmaps", &lightmap_load);

            if (!lightmap_loader) {
                utils_log(LOG_ERROR, "Couldn't create the thread that loads the lightmaps");
                result = 1;
                goto cleanup;
            }
        }

        if (!lightmaps_loaded) {
            if (!thread_get_atomic_int(lightmap_load.finished)) {
                window_clear_surface(window);
                window_draw_text(window, (Vec2) {.x = 0.45, .y = 0.5}, "./assets/fonts/default.ttf", 30.0, "LOADING...");
                window_flip(window);

                time_end_loop = utils_get_current_time_ns();
                continue;
            }

            thread_wait(lightmap_loader);
            lightmap_loader = NULL;

            for (int i = 0; i < max_threads; i++) {
                thread_data[i].walls_light_map = lightmap_load.walls_light_map;
                thread_data[i].floor_light_map = lightmap_load.floor_light_map;
                thread_data[i].ceiling_light_map = lightmap_load.ceiling_light_map;
            }

            lightmaps_loaded = 1;
        }

        Vec2 player_look_at = vec2_from_angle(player.camera.look_at);
        vec2_normalize(&player_look_at);
        vec2_scale(&player_look_at, PLAYER_WALL_COLLISION_CELLS / level->size);
        vec2_add_vec2(&player_look_at, &player.camera.position);

        Vec2 player_look_at_in_map = vec2_map_norm_coord_cp(&player_look_at, level->size, level->size);

        if (map_check_intersection(&level->walls_map, (int)player_look_at_in_map.x, (int)player_look_at_in_map.y) == level->walls_map.wall_empty
            && window_is_key_pressed(KEY_W)) {
            player_move(&player, FORWARD, delta_time);
            vec3_map_norm_coord(&player.light.position, level->size, level->size, 1);
        }

        Vec2 inverted_player_look_at = vec2_from_angle(player.camera.look_at - 180);
        vec2_normalize(&inverted_player_look_at);
        vec2_scale(&inverted_player_look_at, PLAYER_WALL_COLLISION_CELLS / level->size);
        vec2_add_vec2(&inverted_player_look_at, &player.camera.position);

        vec2_map_norm_coord(&inverted_player_look_at, level->size, level->size);

        if (map_check_intersection(&level->walls_map, (int)inverted_player_look_at.x, (int)inverted_player_look_at.y) == level->walls_map.wall_empty &&
            window_is_key_pressed(KEY_S)) {
            player_move(&player, BACKWARDS, delta_time);
            vec3_map_norm_coord(&player.light.position, level->size, level->size, 1);
        }

        if (window_is_key_pressed(KEY_A))
            player_rotate(&player, LEFT, delta_time);
        if (window_is_key_pressed(KEY_D))
            player_rotate(&player, RIGHT, delta_time);

        /**
         *
         * RENDER SCENE 
         *
        */

        window_clear_surface(window);

        for (int i = 0; i < max_threads; i++) {
            thread_signal_semaphore(thread_data[i].start);
        }

        for (int i = 0; i < max_threads; i++) {
            thread_wait_semaphore(thread_data[i].finished);
        }

        sprite_sort(level->sprites, level->sprites_count, &player.camera);

        for (int i = 0; i < level->sprites_count; i++) {
            sprite_render(&level->sprites[i], &player.camera, window, level->size, level->size, z_buffer);
        }

        window_draw_text(window, (Vec2) {.x = 0.01, .y = 0.01}, "./assets/fonts/default.ttf", 30.0, "FPS: %f", 1.0 / fps_count);

        window_flip(window);

        time_end_loop = utils_get_current_time_ns();
    }

cleanup:
    lightmap_loader_stop(&lightmap_load, &lightmap_loader);

    if (lightmap_load.cancel)
        thread_destroy_atomic_int(lightmap_load.cancel);
    if (lightmap_load.finished)
        thread_destroy_atomic_int(lightmap_load.finished);

    if (running != NULL) {
        thread_set_atomic_int(running, 0);

        for (int i = 0; i < max_threads; i++) {
            thread_signal_semaphore(thread_data[i].start);
        }

        for (int i = 0; i < max_threads; i++) {
            thread_wait(threads[i]);

            thread_destroy_semaphore(thread_data[i].start);
            thread_destroy_semaphore(thread_data[i].finished);
        }

        thread_destroy_atomic_int(running);
    }

    if (levels)
        levels_free(levels, levels_count);

    lightmaps_free(&lightmap_load);

    if (world_static_lights)
        free(world_static_lights);

    if (rays)
        free(rays);
    if (z_buffer)
        free(z_buffer);

    if (window)
        window_destroy(window);

    return result;
}

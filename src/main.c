#include <linear_algebra/vec2.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_time.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <player.h>
#include <sdl_utils.h>
#include <stdlib.h>

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 640
#define MIN_DELTA_TIME 0.001

#define PLAYER_LOOK_AT_LENGTH 0.2
#define PLAYER_RECT_WIDTH 0.02

#define CHECK_WALL_STEP 0.01
#define MAP_SIZE 24

typedef enum {
    EMPTY,
    WHITE_WALL,
    BLUE_WALL,
    RED_WALL,
} WallType;

WallType map[MAP_SIZE][MAP_SIZE] = {
    {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 3, 3, 3, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2},
};

void draw_map(SDL_Surface *surface, Player *player) {
    double half_fov = player->fov / 2.0;
    double fov_end_at = player->look_at + half_fov;
    double fov_start_at = player->look_at - half_fov;

    int fov_to_display_ratio = surface->w / player->fov;

    double player_look_at_rads = DEG_TO_RADS(player->look_at);
    double normalized_fov_step = 1.0 / surface->w;
    double current_x = 0.0;

    for (double current_angle = fov_start_at; current_angle < fov_end_at;
         current_angle++) {
        Vec2 ray_start = vec2_map_norm_coord(player->position, MAP_SIZE, MAP_SIZE);
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

        double distance = 0.0;
        WallType wall_hit = EMPTY;

        while (wall_hit == EMPTY && distance < MAP_SIZE) {
            if (ray_length_1d.x < ray_length_1d.y) {
                map_check.x += step.x;
                distance = ray_length_1d.x;
                ray_length_1d.x += ray_unit_step_size.x;
            } else {
                map_check.y += step.y;
                distance = ray_length_1d.y;
                ray_length_1d.y += ray_unit_step_size.y;
            }

            wall_hit = map[(int)map_check.y][(int)map_check.x];
        }

        if (wall_hit != EMPTY) {
            distance *= SDL_cos(DEG_TO_RADS(current_angle) - player_look_at_rads);
            double half_wall_length = ((surface->h / distance) / surface->h) / 2.0;
            RGBA wall_color;

            half_wall_length = half_wall_length > 0.5 ? 0.5 : half_wall_length;

            switch (wall_hit) {
                case WHITE_WALL:
                    wall_color = (RGBA) {
                        .r = 0xFF,
                        .g = 0xFF,
                        .b = 0xFF,
                        .a = 0xFF,
                    };
                    break;
                case BLUE_WALL:
                    wall_color = (RGBA) {
                        .r = 0x1E,
                        .g = 0x3A,
                        .b = 0x8A,
                        .a = 0xFF,
                    };
                break;
                case RED_WALL:
                    wall_color = (RGBA) {
                        .r = 0xCF,
                        .g = 0x00,
                        .b = 0x00,
                        .a = 0xFF,
                    };
                    break;
                default:
                    break;
            }

            for (int x = 0; x <= fov_to_display_ratio; x++) {
                Vec2 wall_start = {
                    .x = current_x,
                    .y = 0.5 - half_wall_length,
                };
                Vec2 wall_end = {
                    .x = current_x,
                    .y = 0.5 + half_wall_length,
                };

                SDLUtils_normalized_FillSurfaceLine(surface, wall_start, wall_end, wall_color);

                current_x += normalized_fov_step;
            }
        }
    }
}

void draw_sky_ground(SDL_Surface *surface, RGBA sky_color, RGBA ground_color) {
    long half_surface_pixels = surface->w * surface->h / 2;

    memset(surface->pixels, SDLUtils_map_rgba(surface, sky_color), half_surface_pixels * sizeof(Uint32));
    memset(surface->pixels + (half_surface_pixels * sizeof(Uint32)), SDLUtils_map_rgba(surface, ground_color), (half_surface_pixels - 1) * sizeof(Uint32));
}

void draw_player(Player *player, SDL_Surface *surface) {
    Vec2 player_sprite_pos = {.x = player->position.x, .y = player->position.y};
    vec2_subtract_double(&player_sprite_pos, (PLAYER_RECT_WIDTH / 2.0));

    SDLUtils_normalized_FillSurfaceCircle(
        surface, 0.02, player->position,
        (RGBA){.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF});

    Vec2 look_at = vec2_from_angle(player->look_at);
    vec2_scale(&look_at, PLAYER_LOOK_AT_LENGTH);
    vec2_add_vec2(&look_at, player->position);

    SDLUtils_normalized_FillSurfaceLine(
        surface, player->position, look_at,
        (RGBA){.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF});

    double half_fov = player->fov / 2.0;
    double fov_start_at = player->look_at - half_fov;
    double fov_end_at = player->look_at + half_fov;

    for (double current_angle = fov_start_at; current_angle < fov_end_at;
         current_angle++) {
        int wall_hit = 0;

        Vec2 ray_start = vec2_map_norm_coord(player->position, MAP_SIZE, MAP_SIZE);
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

        double distance = 0.0;
        while (!wall_hit && distance < MAP_SIZE) {
            if (ray_length_1d.x < ray_length_1d.y) {
                map_check.x += step.x;
                distance = ray_length_1d.x;
                ray_length_1d.x += ray_unit_step_size.x;
            } else {
                map_check.y += step.y;
                distance = ray_length_1d.y;
                ray_length_1d.y += ray_unit_step_size.y;
            }

            switch (map[(int)map_check.y][(int)map_check.x]) {
            case WHITE_WALL:
                wall_hit = 1;
                    Vec2 intersection = ray_start;
                    vec2_scale(&ray_dir, distance);
                    vec2_add_vec2(&intersection, ray_dir);
                    vec2_scale(&intersection, 1.0 / MAP_SIZE);

                SDLUtils_normalized_FillSurfaceCircle(surface, 0.03, intersection,
                                                      (RGBA){.r = 0xFF});
                break;
            default:
                break;
            }
        }
    }
}

int main(void) {
    SDL_SetAppMetadata("Ray Caster", "1.0", "com.ray_caster");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window *window =
        SDL_CreateWindow("Ray Caster", WINDOW_WIDTH, WINDOW_HEIGHT,
                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (!window) {
        SDL_Log("Couldn't create the application's window");
        return SDL_APP_FAILURE;
    }

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    Player player = {.position = {.x = 0.5, .y = 0.5},
                     .look_at = 0.0,
                     .fov = 90.0,
                     .movement_speed = 0.8,
                     .rotation_speed = 500.0};
    Vec2 old_player_pos = player.position;

    SDL_Event event;

    int keep_window_open = 1;

    SDL_Time time_start_loop = 0;
    SDL_Time time_end_loop = 0;
    double delta_time = 0.0;

    while (keep_window_open) {
        SDL_GetCurrentTime(&time_start_loop);
        delta_time = NS_TO_S(time_start_loop - time_end_loop) + MIN_DELTA_TIME;
        SDL_Log("FPS: %f", 1.0 / delta_time);

        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                keep_window_open = 0;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                if (!SDL_GetWindowSize(window, NULL, NULL)) {
                    SDL_Log("Panicked getting window size: %s", SDL_GetError());
                    return SDL_APP_FAILURE;
                }

                surface = SDL_GetWindowSurface(window);
                break;
            }
        }

        const bool *key_states = SDL_GetKeyboardState(NULL);

        old_player_pos = player.position;

        if (key_states[SDL_SCANCODE_W])
            player_move(&player, FORWARD, delta_time);
        if (key_states[SDL_SCANCODE_S])
            player_move(&player, BACKWARDS, delta_time);
        if (key_states[SDL_SCANCODE_A])
            player_rotate(&player, LEFT, delta_time);
        if (key_states[SDL_SCANCODE_D])
            player_rotate(&player, RIGHT, delta_time);

        Vec2 player_pos_in_map = vec2_map_norm_coord(player.position, MAP_SIZE, MAP_SIZE);
        if (map[(int)player_pos_in_map.y][(int)player_pos_in_map.x] != EMPTY)
            player.position = old_player_pos;

        SDL_ClearSurface(surface, 0x00, 0x00, 0x00, 0xFF);

        draw_sky_ground(surface, (RGBA) { 0x57, 0x57, 0x57, 0xFF }, (RGBA) { 0x71, 0x71, 0x71, 0xFF });
        draw_map(surface, &player);

        SDL_UpdateWindowSurface(window);

        SDL_GetCurrentTime(&time_end_loop);
    }

    SDL_DestroyWindow(window);
    SDL_DestroySurface(surface);
    SDL_Quit();

    return 0;
}

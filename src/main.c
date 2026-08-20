#include "linear_algebra/vec2.h"
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
#define MAP_SIZE 10

typedef enum {
    Empty,
    Wall,
} WallType;

WallType map[MAP_SIZE][MAP_SIZE] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}, {1, 0, 1, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}, {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

void draw_map(SDL_Surface *surface) {
    double square_side_len = 1.0 / MAP_SIZE;
    double line_pos = 0.0;
    Vec2 line_start = {.x = 0.0, .y = 0.0};
    Vec2 line_end = {.x = 0.0, .y = 0.0};

    for (int x = 0; x < MAP_SIZE; x++) {
        line_pos = square_side_len * x;

        line_start = (Vec2){.x = line_pos, .y = 0.0};
        line_end = (Vec2){.x = line_pos, .y = 1.0};

        SDLUtils_normalized_FillSurfaceLine(
            surface, line_start, line_end,
            (RGBA){.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF});
    }

    for (int y = 0; y < MAP_SIZE; y++) {
        line_pos = square_side_len * y;

        line_start = (Vec2){.x = 0.0, .y = line_pos};
        line_end = (Vec2){.x = 1.0, .y = line_pos};

        SDLUtils_normalized_FillSurfaceLine(
            surface, line_start, line_end,
            (RGBA){.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF});
    }

    Vec2 square_pos;
    Vec2 square_size;

    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            switch (map[i][j]) {
            case Wall:
                square_pos =
                    (Vec2){.x = square_side_len * j, .y = square_side_len * i};
                square_size =
                    (Vec2){.x = square_side_len, .y = square_side_len};

                SDLUtils_normalized_FillSurfaceRect(
                    surface, square_pos, square_size,
                    (RGBA){.r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0xFF});
                break;
            default:
                break;
            }
        }
    }
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
        Vec2 ray = vec2_from_angle(current_angle);

        vec2_scale(&ray, CHECK_WALL_STEP);

        Vec2 ray_dir = ray;

        vec2_add_vec2(&ray, player->position);

        while ((ray.x > 0.0 && ray.y > 0.0) && (ray.x < 1.0 && ray.y < 1.0) &&
               !wall_hit) {
            vec2_add_vec2(&ray, ray_dir);
            Vec2 ray_map_pos = vec2_map_norm_coord(ray, MAP_SIZE, MAP_SIZE);

            switch (map[(int)ray_map_pos.y][(int)ray_map_pos.x]) {
            case Wall:
                wall_hit = 1;
                SDLUtils_normalized_FillSurfaceCircle(surface, 0.03, ray,
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
                    exit(1);
                }

                surface = SDL_GetWindowSurface(window);
                break;
            }
        }

        const bool *key_states = SDL_GetKeyboardState(NULL);

        if (key_states[SDL_SCANCODE_W])
            player_move(&player, FORWARD, delta_time);
        if (key_states[SDL_SCANCODE_S])
            player_move(&player, BACKWARDS, delta_time);
        if (key_states[SDL_SCANCODE_A])
            player_rotate(&player, LEFT, delta_time);
        if (key_states[SDL_SCANCODE_D])
            player_rotate(&player, RIGHT, delta_time);

        SDL_ClearSurface(surface, 0x00, 0x00, 0x00, 0xFF);

        draw_map(surface);
        draw_player(&player, surface);

        SDL_UpdateWindowSurface(window);

        SDL_GetCurrentTime(&time_end_loop);
    }

    SDL_DestroyWindow(window);
    SDL_DestroySurface(surface);
    SDL_Quit();

    return 0;
}

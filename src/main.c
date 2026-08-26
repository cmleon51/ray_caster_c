#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
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
#include <map.h>

#define MAP_SIZE 24

enum WallType{
    EMPTY,
    WHITE_WALL,
    BLUE_WALL,
    RED_WALL,
};

WallType map_2d[MAP_SIZE][MAP_SIZE] = {
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

RGBA get_wall_type_color(WallType wall, SIDE_HIT side_hit) {
    RGBA wall_color = {};

    switch (wall) {
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

    if (side_hit == Y_SIDE) {
        wall_color.r = wall_color.r >> 1;
        wall_color.g = wall_color.g >> 1;
        wall_color.b = wall_color.b >> 1;
    }

    return wall_color;
}

void draw_sky_ground(SDL_Surface *surface, RGBA sky_color, RGBA ground_color) {
    long half_surface_pixels = surface->w * surface->h / 2;

    SDL_memset(surface->pixels, SDLUtils_map_rgba(surface, sky_color), half_surface_pixels * sizeof(Uint32));
    SDL_memset(surface->pixels + (half_surface_pixels * sizeof(Uint32)), SDLUtils_map_rgba(surface, ground_color), (half_surface_pixels - 1) * sizeof(Uint32));
}

int main(void) {
    SDL_SetAppMetadata("Ray Caster", "1.0", "com.ray_caster");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // specify 0, 0 in window width and height to let the window manager decide the window's size
    SDL_Window *window =
        SDL_CreateWindow("Ray Caster", 0, 0,
                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (!window) {
        SDL_Log("Couldn't create the application's window");
        return SDL_APP_FAILURE;
    }

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    Player player = {.position = {.x = 0.5, .y = 0.5},
                     .look_at = 0.0,
                     .fov = 90.0,
                     .movement_speed = 0.4,
                     .rotation_speed = 200.0};
    Vec2 old_player_pos = player.position;

    Map map;
    map_create(&map, &player, MAP_SIZE, MAP_SIZE, &map_2d[0][0], EMPTY, get_wall_type_color);

    SDL_Event event;

    int keep_window_open = 1;

    SDL_Time time_start_loop = 0;
    SDL_Time prev_time_start_loop = 0;
    SDL_Time time_end_loop = 0;
    double delta_time = 0.0;
    double fps_count = 0.0;

    while (keep_window_open) {
        fps_count = NS_TO_S(time_end_loop - time_start_loop);
        SDL_GetCurrentTime(&time_start_loop);

        delta_time = NS_TO_S(time_start_loop - prev_time_start_loop);

        SDL_GetCurrentTime(&prev_time_start_loop);

        SDL_Log("FPS: %f", 1.0 / fps_count);

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

        Vec2 player_pos_in_map = vec2_map_norm_coord(player.position, map.map_width, map.map_height);
        if (map_check_wall(&map, (int)player_pos_in_map.x, (int)player_pos_in_map.y) != EMPTY)
            player.position = old_player_pos;

        SDL_ClearSurface(surface, 0x00, 0x00, 0x00, 0xFF);

        draw_sky_ground(surface, (RGBA) { 0x57, 0x57, 0x57, 0xFF }, (RGBA) { 0x71, 0x71, 0x71, 0xFF });
        map_draw(&map, surface);

        SDL_UpdateWindowSurface(window);

        SDL_GetCurrentTime(&time_end_loop);
    }

    map_delete(&map);

    SDL_DestroyWindow(window);
    SDL_DestroySurface(surface);
    SDL_Quit();

    return 0;
}

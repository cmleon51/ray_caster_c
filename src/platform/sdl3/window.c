#include <platform/window.h>
#include <stdlib.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

struct Window {
    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Event event;
    int window_should_quit;
};

Window *window_create() {
    Window *window = (Window *)calloc(1, sizeof(Window));
    SDL_SetAppMetadata("Ray Caster", "1.0", "com.ray_caster");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return NULL;
    }

    window->window =
        SDL_CreateWindow("Ray Caster", 0, 0,
                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (!window) {
        SDL_Log("Couldn't create the application's window");
        return NULL;
    }

    window->surface = SDL_GetWindowSurface(window->window);

    return window;
}

void window_flip(Window *window) {
    SDL_UpdateWindowSurface(window->window);
}

void window_clear_surface(Window *window) {
    SDL_ClearSurface(window->surface, 0x00, 0x00, 0x00, 0x00);
}

int window_resize(Window *window) {
    if (!SDL_GetWindowSize(window->window, NULL, NULL)) {
        SDL_Log("Panicked getting window size: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window->surface = SDL_GetWindowSurface(window->window);

    return 0;
}

int window_get_width(Window *window) {
    return window->surface->w;
}

int window_get_height(Window *window) {
    return window->surface->h;
}

int window_should_quit(Window *window) {
    return window->window_should_quit;
}

Event window_poll_event(Window *window) {
    if (SDL_PollEvent(&window->event) > 0) {
        switch (window->event.type) {
            case SDL_EVENT_QUIT:
                return EVENT_QUIT_APP;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                return EVENT_WINDOW_RESIZED;
                break;
        }
    }

    return EVENT_NONE;
}

int window_is_key_pressed(KeyPress key) {
    const bool *key_states = SDL_GetKeyboardState(NULL);

    switch (key) {
        case KEY_W:
            return key_states[SDL_SCANCODE_W];
            break;
        case KEY_A:
            return key_states[SDL_SCANCODE_A];
            break;
        case KEY_S:
            return key_states[SDL_SCANCODE_S];
            break;
        case KEY_D:
            return key_states[SDL_SCANCODE_D];
            break;
        default:
            return 0;
            break;
    }
}

void window_draw_line(Window *window, Vec2 norm_start, Vec2 norm_end, RGBA *colors, int colors_count) {
    SDL_Surface *window_surface = window->surface;

    Vec2 end = vec2_map_norm_coord(norm_end, window_surface->w, window_surface->h);
    Vec2 start = vec2_map_norm_coord(norm_start, window_surface->w, window_surface->h);

    end = (Vec2) {
        .x = SDL_round(end.x),
        .y = SDL_round(end.y)
    };
    start = (Vec2) {
        .x = SDL_round(start.x),
        .y = SDL_round(start.y)
    };

    if ((int)end.x >= window_surface->w) {
        end.x = window_surface->w - 1;
    }
    if ((int)end.y >= window_surface->h) {
        end.y = window_surface->h - 1;
    }
    if ((int)start.x < 0) {
        start.x = 0;
    }
    if ((int)start.y < 0) {
        start.y = 0;
    }

    Vec2 ray_dir = end;
    vec2_subtract_vec2(&ray_dir, start);

    double steps = SDL_abs(ray_dir.x) > SDL_abs(ray_dir.y) ? SDL_abs(ray_dir.x) : SDL_abs(ray_dir.y);

    double x_inc = ray_dir.x / steps;
    double y_inc = ray_dir.y / steps;

    int current_color_index = 0;

    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(window_surface->format);
    int bpp = format->bytes_per_pixel;

    if (SDL_MUSTLOCK(window_surface))
        SDL_LockSurface(window_surface);

    RGBA *prev_color = &colors[current_color_index];
    Uint32 pixel = SDL_MapRGBA(format, NULL, prev_color->r, prev_color->g, prev_color->b, prev_color->a);

    for (int i = 0; i <= steps; i++) {
        int x = (int)start.x;
        int y = (int)start.y;

        RGBA *current_color = &colors[current_color_index];

        if (current_color->r != prev_color->r || current_color->g != prev_color->g ||
            current_color->b != prev_color->b || current_color->a != prev_color->a) {
            pixel = SDL_MapRGBA(format, NULL, current_color->r, current_color->g, current_color->b, current_color->a);
            prev_color = current_color;
        }

        Uint8 *dst = (Uint8 *)window_surface->pixels + y * window_surface->pitch + x * bpp;
        SDL_memcpy(dst, &pixel, bpp);

        start.x += x_inc;
        start.y += y_inc;

        if (current_color_index < colors_count - 1)
            current_color_index++;
    }

    if (SDL_MUSTLOCK(window_surface))
        SDL_UnlockSurface(window_surface);
}

void window_close(Window *window) {
    window->window_should_quit = 1;
}

void window_destroy(Window *window) {
    SDL_DestroySurface(window->surface);
    SDL_DestroyWindow(window->window);

    SDL_Quit();
}

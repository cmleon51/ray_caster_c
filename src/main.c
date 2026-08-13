#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 640

int main(int argc, char **argv) {
    SDL_SetAppMetadata("Ray Caster", "1.0",
                       "com.ray_caster");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window *window =
        SDL_CreateWindow("Ray Caster", WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if (!window) {
        SDL_Log("Couldn't create the application's window");
        return SDL_APP_FAILURE;
    }

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_Event event;

    int keep_window_open = 1;

    while (keep_window_open) {
        SDL_ClearSurface(surface, 0, 0, 0, 255);
        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                keep_window_open = 0;
                break;
            }
        }

        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_DestroySurface(surface);
    SDL_Quit();

    return 0;
}

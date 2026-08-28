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
#include <stb_image.h>
#include <textures.h>

#define MAP_SIZE 50
#define TEXTURE_COUNT 8

Texture *textures;

const char *texture_paths[TEXTURE_COUNT] = {
    "./assets/textures/gold_ore.png",
    "./assets/textures/pale_stone.png",
    "./assets/textures/crystal_panel.png",
    "./assets/textures/slate_masonry.png",
    "./assets/textures/dark_alcove.png",
    "./assets/textures/white_brick.png",
    "./assets/textures/charcoal_brick.png",
    "./assets/textures/rust_brick.png",
};

int load_textures() {
    textures = malloc(sizeof(Texture) * TEXTURE_COUNT);
    const char *failure = NULL;

    for (int i = 0; i < TEXTURE_COUNT; i++) {
        int load_texture_result = texture_load(&textures[i], texture_paths[i], &failure);

        if (load_texture_result == 1) {
            SDL_Log("Couldn't load the texture at %s for the following reason: %s", texture_paths[i], failure);
            return 1;
        }
    }

    return 0;
}

void free_textures() {
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        texture_destroy(&textures[i]);
    }

    free(textures);
}

enum WallType{
    EMPTY,
    TEXTURE_GOLD_ORE,
    TEXTURE_PALE_STONE,
    TEXTURE_CRYSTAL_PANEL,
    TEXTURE_SLATE_MASONRY,
    TEXTURE_DARK_ALCOVE,
    TEXTURE_WHITE_BRICK,
    TEXTURE_CHARCOAL_BRICK,
    TEXTURE_RUST_BRICK,
};

// Wall ids index texture_paths[] as textures[wall - 1], so this order must
// match texture_paths above.
//   0 empty       3 crystal_panel   6 white_brick
//   1 gold_ore    4 slate_masonry   7 charcoal_brick
//   2 pale_stone  5 dark_alcove     8 rust_brick
WallType map_2d[MAP_SIZE][MAP_SIZE] = {
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 5, 5, 0, 5, 5, 5, 4, 5, 5, 0, 5, 5, 5, 4, 5, 5, 0, 5, 5, 5, 4, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 1, 1, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 5, 5, 0, 5, 5, 5, 4, 5, 5, 0, 5, 5, 5, 4, 5, 5, 0, 5, 5, 5, 4, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 4, 1, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 4, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 8, 3, 0, 0, 3, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 8, 0, 0, 0, 0, 0, 0, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 4},
    {4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 0, 0, 0, 0, 0, 0, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 7, 0, 0, 7, 4, 4, 4, 4, 4, 4, 4, 3, 0, 0, 0, 0, 0, 0, 3, 4, 4, 6, 0, 0, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4},
    {4, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 8, 0, 0, 0, 0, 0, 0, 8, 6, 6, 6, 0, 0, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 8, 3, 0, 0, 3, 8, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 0, 0, 7, 7, 7, 7, 7, 7, 0, 0, 0, 7, 4, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 7, 2, 0, 0, 2, 7, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 4, 6, 6, 6, 6, 0, 0, 6, 6, 6, 4, 8, 8, 8, 8, 0, 0, 8, 8, 8, 8, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 7, 2, 2, 2, 2, 7, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
};

typedef struct {
    int thread_nr;
    int max_threads;
    Player *player;
    RayHit **rays_arr;
    Map *map;
    SDL_Surface **surface;
    SDL_AtomicInt *running;
    SDL_Semaphore *start;
    SDL_Semaphore *finished;
} ThreadData;

int draw_map_portion(void *args) {
    ThreadData *data = (ThreadData *)args;

    while (SDL_GetAtomicInt(data->running)) {
        SDL_WaitSemaphore(data->start);

        int width_threads_ratio = (*data->surface)->w / data->max_threads;
        int column_start = width_threads_ratio * data->thread_nr;
        int column_end = width_threads_ratio + column_start;

        if (data->thread_nr == data->max_threads - 1) {
            column_end = (*data->surface)->w;
        }

        map_raycast(data->map, *data->rays_arr, data->player, column_start, column_end, (*data->surface)->w, (*data->surface)->h);

        for (int i = column_start; i < column_end; i++) {
            RayHit *curr_ray = &(*data->rays_arr)[i];
            SDL_Color wall_colors[curr_ray->wall_height];
            Texture *current_texture = &textures[curr_ray->wall_hit - 1];

            double texture_wall_height_ratio = (double)current_texture->height / curr_ray->full_wall_height;
            int texture_x = curr_ray->wall_column_hit * current_texture->width;

            for (int y = 0; y < curr_ray->wall_height; y++) {
                int texture_y = (y + curr_ray->wall_height_clip) * texture_wall_height_ratio;

                wall_colors[y] = texture_get_pixel(current_texture, texture_x, texture_y);

                if (curr_ray->side_hit == Y_SIDE) {
                    wall_colors[y].r = wall_colors[y].r >> 1;
                    wall_colors[y].g = wall_colors[y].g >> 1;
                    wall_colors[y].b = wall_colors[y].b >> 1;
                }
            }

            SDLUtils_normalized_FillSurfaceLine(*data->surface, curr_ray->wall_start, curr_ray->wall_end, wall_colors, curr_ray->wall_height);
        }

        SDL_SignalSemaphore(data->finished);
    }

    return 0;
}

void draw_sky_ground(SDL_Surface *surface, SDL_Color sky_color, SDL_Color ground_color) {
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

    int load_texture_result = load_textures();

    if (load_texture_result == 1) {
        free_textures();
        return load_texture_result;
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
                     .movement_speed = 0.2,
                     .rotation_speed = 200.0};
    double player_wall_collision_distance = 0.01;

    Map map = {
        .map_width = MAP_SIZE,
        .map_height = MAP_SIZE,
        .map_2d = &map_2d[0][0],
        .wall_empty = EMPTY
    };
    RayHit *rays = malloc(sizeof(RayHit) * surface->w);

    int max_threads = SDL_GetNumLogicalCPUCores();
    ThreadData thread_data[max_threads];
    SDL_Thread *threads[max_threads];
    SDL_AtomicInt *running = (SDL_AtomicInt *)malloc(sizeof(SDL_AtomicInt));

    SDL_SetAtomicInt(running, 1);

    for (int i = 0; i < max_threads; i++) {
        SDL_Semaphore *start_semaphore = SDL_CreateSemaphore(0);
        SDL_Semaphore *finished_semaphore = SDL_CreateSemaphore(0);

        thread_data[i] = (ThreadData) {
            .thread_nr = i,
            .max_threads = max_threads,
            .player = &player,
            .rays_arr = &rays,
            .map = &map,
            .surface = &surface,
            .running = running,
            .start = start_semaphore,
            .finished = finished_semaphore,
        };

        threads[i] = SDL_CreateThread(draw_map_portion, "thread", &thread_data[i]);
    }

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
                rays = realloc(rays, sizeof(RayHit) * surface->w);
                break;
            }
        }

        const bool *key_states = SDL_GetKeyboardState(NULL);

        Vec2 player_look_at = vec2_from_angle(player.look_at);
        vec2_normalize(&player_look_at);
        vec2_scale(&player_look_at, player_wall_collision_distance);
        vec2_add_vec2(&player_look_at, player.position);

        Vec2 player_look_at_in_map = vec2_map_norm_coord(player_look_at, map.map_width, map.map_height);

        if (map_check_wall(&map, (int)player_look_at_in_map.x, (int)player_look_at_in_map.y) == EMPTY
            && key_states[SDL_SCANCODE_W]) {
            player_move(&player, FORWARD, delta_time);
        }

        Vec2 inverted_player_look_at = vec2_from_angle(player.look_at - 180);
        vec2_normalize(&inverted_player_look_at);
        vec2_scale(&inverted_player_look_at, player_wall_collision_distance);
        vec2_add_vec2(&inverted_player_look_at, player.position);

        Vec2 inverted_player_look_at_in_map = vec2_map_norm_coord(inverted_player_look_at, map.map_width, map.map_height);

        if (map_check_wall(&map, (int)inverted_player_look_at_in_map.x, (int)inverted_player_look_at_in_map.y) == EMPTY
            && key_states[SDL_SCANCODE_S]) {
            player_move(&player, BACKWARDS, delta_time);
        }

        if (key_states[SDL_SCANCODE_A])
            player_rotate(&player, LEFT, delta_time);
        if (key_states[SDL_SCANCODE_D])
            player_rotate(&player, RIGHT, delta_time);


        SDL_ClearSurface(surface, 0x00, 0x00, 0x00, 0xFF);

        draw_sky_ground(surface, (SDL_Color) { 0x57, 0x57, 0x57, 0xFF }, (SDL_Color) { 0x71, 0x71, 0x71, 0xFF });

        for (int i = 0; i < max_threads; i++) {
            SDL_SignalSemaphore(thread_data[i].start);
        }

        for (int i = 0; i < max_threads; i++) {
            SDL_WaitSemaphore(thread_data[i].finished);
        }

        SDL_UpdateWindowSurface(window);

        SDL_GetCurrentTime(&time_end_loop);
    }

    SDL_SetAtomicInt(running, 0);

    for (int i = 0; i < max_threads; i++) {
        SDL_SignalSemaphore(thread_data[i].start);
    }

    for (int i = 0; i < max_threads; i++) {
        SDL_WaitThread(threads[i], NULL);

        SDL_DestroySemaphore(thread_data[i].start);
        SDL_DestroySemaphore(thread_data[i].finished);
    }

    free(running);
    free(rays);
    free_textures();

    SDL_DestroyWindow(window);
    SDL_DestroySurface(surface);
    SDL_Quit();

    return 0;
}

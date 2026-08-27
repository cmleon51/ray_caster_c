#ifndef ASSETS_H
#define ASSETS_H

#include <SDL3/SDL_pixels.h>

typedef struct {
    int height;
    int width;
    SDL_Color *pixels;
} Texture;

int texture_load(Texture *texture, const char *texture_path, const char **failure);

SDL_Color texture_get_pixel(Texture *texture, int x, int y);

void texture_destroy(Texture *texture);

#endif // ASSETS_H

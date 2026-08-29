#include <textures.h>

#define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>

int texture_load(Texture *texture, const char *texture_path, const char **failure) {
    int channels;
    unsigned char *image = stbi_load(texture_path, &texture->width, &texture->height, &channels, 0);

    if (image == NULL) {
        *failure = stbi_failure_reason();
        return 1;
    }

    texture->pixels = malloc(sizeof(SDL_Color) * texture->width * texture->height);

    for (int y = 0; y < texture->height; y++) {
        for (int x = 0; x < texture->width; x++) {
            unsigned char *pixel = image + (y * texture->width + x) * channels;
            unsigned char r = pixel[0];
            unsigned char g = (channels > 1) ? pixel[1] : r;
            unsigned char b = (channels > 2) ? pixel[2] : g;
            unsigned char a = (channels > 3) ? pixel[3] : 255;

            texture->pixels[y * texture->width + x] = (SDL_Color) {
                .r = r,
                .g = g,
                .b = b,
                .a = a
            };
        }
    }

    stbi_image_free(image);

    return 0;
}

extern SDL_Color texture_get_pixel(Texture *texture, int x, int y);

void texture_destroy(Texture *texture) {
    free(texture->pixels);
}

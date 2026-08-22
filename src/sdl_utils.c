#include "vec2.h"
#include <sdl_utils.h>

Uint32 SDLUtils_map_rgba(SDL_Surface *surface, RGBA color) {
    const SDL_PixelFormatDetails *surface_format =
        SDL_GetPixelFormatDetails(surface->format);
    return SDL_MapRGBA(surface_format, NULL, color.r, color.g, color.b,
                       color.a);
}

void SDLUtils_normalized_FillSurfaceRect(SDL_Surface *surface,
                                         Vec2 norm_rect_pos,
                                         Vec2 norm_rect_size, RGBA color) {
    Vec2 rect_pos = vec2_map_norm_coord(norm_rect_pos, surface->w, surface->h);
    Vec2 rect_size = vec2_map_norm_coord(norm_rect_size, surface->w, surface->h);

    SDL_Rect rect = {
        .x = SDL_round(rect_pos.x),
        .y = SDL_round(rect_pos.y),
        .w = SDL_round(rect_size.x),
        .h = SDL_round(rect_size.y),
    };

    SDL_FillSurfaceRect(surface, &rect, SDLUtils_map_rgba(surface, color));
}

void SDLUtils_normalized_WriteSurfacePixel(SDL_Surface *surface,
                                           Vec2 norm_pixel_pos, RGBA color) {
    Vec2 pixel_pos = vec2_map_norm_coord(norm_pixel_pos, surface->w, surface->h);

    SDL_WriteSurfacePixel(surface, pixel_pos.x, pixel_pos.y, color.r, color.g,
                          color.b, color.a);
}

void SDLUtils_normalized_FillSurfaceLine(SDL_Surface *surface, Vec2 norm_start,
                                         Vec2 norm_end, RGBA color) {
    Vec2 end = vec2_map_norm_coord(norm_end, surface->w, surface->h);
    Vec2 start = vec2_map_norm_coord(norm_start, surface->w, surface->h);

     end = (Vec2) {
         .x = SDL_round(end.x),
         .y = SDL_round(end.y)
     };
     start = (Vec2) {
         .x = SDL_round(start.x),
         .y = SDL_round(start.y)
     };

    Vec2 ray_dir = end;
    vec2_subtract_vec2(&ray_dir, start);

    int steps = SDL_abs(ray_dir.x) > SDL_abs(ray_dir.y) ? SDL_abs(ray_dir.x) : SDL_abs(ray_dir.y);

    double x_inc = ray_dir.x / (double)steps;
    double y_inc = ray_dir.y / (double)steps;

    for (int i = 0; i <= steps; i++) {
        SDL_WriteSurfacePixel(surface, start.x, start.y, color.r, color.g, color.b,
                              color.a);

        start.x += x_inc;
        start.y += y_inc;
    }
}

void SDLUtils_normalized_FillSurfaceCircle(SDL_Surface *surface,
                                           double norm_radius,
                                           Vec2 norm_position, RGBA color) {
    Vec2 position = vec2_map_norm_coord(norm_position, surface->w, surface->h);
    double radius = SDL_round(norm_radius * surface->w);

    position = (Vec2) {
        .x = SDL_round(position.x),
        .y = SDL_round(position.y)
    };

    double t1 = radius / 16.0;
    double t2 = 0.0;
    int x = radius;
    int y = 0;

    while (x >= y) {
        SDL_WriteSurfacePixel(surface, position.x + x, position.y + y, color.r,
                              color.g, color.b, color.a);
        SDL_WriteSurfacePixel(surface, position.x - x, position.y - y, color.r,
                              color.g, color.b, color.a);

        SDL_WriteSurfacePixel(surface, position.x + x, position.y - y, color.r,
                              color.g, color.b, color.a);
        SDL_WriteSurfacePixel(surface, position.x - x, position.y + y, color.r,
                              color.g, color.b, color.a);

        SDL_WriteSurfacePixel(surface, position.x + y, position.y + x, color.r,
                              color.g, color.b, color.a);
        SDL_WriteSurfacePixel(surface, position.x - y, position.y - x, color.r,
                              color.g, color.b, color.a);

        SDL_WriteSurfacePixel(surface, position.x + y, position.y - x, color.r,
                              color.g, color.b, color.a);
        SDL_WriteSurfacePixel(surface, position.x - y, position.y + x, color.r,
                              color.g, color.b, color.a);

        y = y + 1;
        t1 = t1 + y;
        t2 = t1 - x;

        if (t2 >= 0) {
            t1 = t2;
            x = x - 1;
        }
    }
}

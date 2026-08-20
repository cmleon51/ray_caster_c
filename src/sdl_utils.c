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

// using Bresenham's line algorithm
void SDLUtils_FillSurfaceLineH(SDL_Surface *surface, Vec2 start, Vec2 end,
                               RGBA color) {
    if (start.x > end.x) {
        double tmp = start.x;
        start.x = end.x;
        end.x = tmp;

        tmp = start.y;
        start.y = end.y;
        end.y = tmp;
    }

    Vec2 dvec = end;
    vec2_subtract_vec2(&dvec, start);

    int dir = dvec.y < 0 ? -1 : 1;
    dvec.y *= dir;

    if (dvec.x != 0) {
        int y = start.y;
        int derivative = 2 * dvec.y - dvec.x;

        for (int x = start.x; x < end.x; x++) {
            SDL_WriteSurfacePixel(surface, x, y, color.r, color.g, color.b,
                                  color.a);

            if (derivative >= 0) {
                y += dir;
                derivative -= 2 * dvec.x;
            }

            derivative += 2 * dvec.y;
        }
    }
}

void SDLUtils_FillSurfaceLineV(SDL_Surface *surface, Vec2 start, Vec2 end,
                               RGBA color) {
    if (start.y > end.y) {
        double tmp = start.x;
        start.x = end.x;
        end.x = tmp;

        tmp = start.y;
        start.y = end.y;
        end.y = tmp;
    }

    Vec2 dvec = end;
    vec2_subtract_vec2(&dvec, start);

    int dir = dvec.x < 0 ? -1 : 1;
    dvec.x *= dir;

    if (dvec.y != 0) {
        int x = start.x;
        int derivative = 2 * dvec.x - dvec.y;

        for (int y = start.y; y < end.y; y++) {
            SDL_WriteSurfacePixel(surface, x, y, color.r, color.g, color.b,
                                  color.a);

            if (derivative >= 0) {
                x += dir;
                derivative -= 2 * dvec.y;
            }

            derivative += 2 * dvec.x;
        }
    }
}

void SDLUtils_normalized_FillSurfaceLine(SDL_Surface *surface, Vec2 norm_start,
                                         Vec2 norm_end, RGBA color) {
    Vec2 end = vec2_map_norm_coord(norm_end, surface->w, surface->h);
    Vec2 start = vec2_map_norm_coord(norm_start, surface->w, surface->h);

    if (SDL_abs(end.x - start.x) > SDL_abs(end.y - start.y))
        SDLUtils_FillSurfaceLineH(surface, start, end, color);
    else
        SDLUtils_FillSurfaceLineV(surface, start, end, color);
}

void SDLUtils_normalized_FillSurfaceCircle(SDL_Surface *surface,
                                           double norm_radius,
                                           Vec2 norm_position, RGBA color) {
    Vec2 position = vec2_map_norm_coord(norm_position, surface->w, surface->h);
    double radius = norm_radius * surface->w;

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

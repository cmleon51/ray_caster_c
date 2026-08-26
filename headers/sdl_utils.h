#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <linear_algebra/vec2.h>
#include <player.h>

#define NS_TO_S(nano_seconds) ((nano_seconds) * 1e-9f)

Uint32 SDLUtils_map_rgba(SDL_Surface *surface, SDL_Color color);

void SDLUtils_normalized_FillSurfaceRect(SDL_Surface *surface, Vec2 norm_rect_pos,
                                    Vec2 norm_rect_size, SDL_Color color);

void SDLUtils_normalized_WriteSurfacePixel(SDL_Surface *surface, Vec2 norm_pixel_pos,
                                      SDL_Color color);

void SDLUtils_normalized_FillSurfaceCircle(SDL_Surface *surface, double norm_radius,
                                      Vec2 norm_position, SDL_Color color);

void SDLUtils_normalized_FillSurfaceLine(SDL_Surface *surface, Vec2 norm_start,
                                    Vec2 norm_end, SDL_Color *colors, int colors_count);

#endif

#ifndef PLAYER_H
#define PLAYER_H

#include "linear_algebra/vec2.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>

typedef struct {
    Vec2 position;
    double look_at;
    double movement_speed;
    double rotation_speed;
} Player;

typedef enum { FORWARD, BACKWARDS, LEFT, RIGHT } Direction;

void player_move(Player *player, Direction direction, double delta_time);

void player_rotate(Player *player, Direction direction, double delta_time);

#endif

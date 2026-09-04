#ifndef PLAYER_H
#define PLAYER_H

#include <linear_algebra/vec2.h>
#include <camera.h>

typedef struct {
    Camera camera;
    double movement_speed;
    double rotation_speed;
} Player;

typedef enum { FORWARD, BACKWARDS, LEFT, RIGHT } Direction;

void player_move(Player *player, Direction direction, double delta_time);

void player_rotate(Player *player, Direction direction, double delta_time);

#endif

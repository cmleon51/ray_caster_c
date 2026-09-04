#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <stdlib.h>

#include <player.h>

void player_move(Player *player, Direction direction, double delta_time) {
    Vec2 look_at_vector = vec2_from_angle(player->camera.look_at);
    vec2_modify_length(&look_at_vector, player->movement_speed * delta_time);

    switch (direction) {
    case FORWARD:
        vec2_add_vec2(&player->camera.position, look_at_vector);
        break;
    case BACKWARDS:
        vec2_subtract_vec2(&player->camera.position, look_at_vector);
        break;
    default:
        SDL_Log("The provided direction: %d is not yet implemented in function %s",
                direction, __FUNCTION__);
        exit(SDL_APP_FAILURE);
        break;
    };
}

void player_rotate(Player *player, Direction direction, double delta_time) {
    switch (direction) {
    case LEFT:
        player->camera.look_at -= player->rotation_speed * delta_time;
        break;
    case RIGHT:
        player->camera.look_at += player->rotation_speed * delta_time;
        break;
    default:
        SDL_Log("The provided direction: %d is not yet implemented in function %s",
                direction, __FUNCTION__);
        exit(SDL_APP_FAILURE);
        break;
    }
}

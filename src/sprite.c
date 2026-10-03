#include <sprite.h>

#include <math.h>
#include <stdlib.h>

void sprite_sort(Sprite *sprites_array, int sprites_count, Camera *camera) {
    // sort sprites by distance from player
    for (int i = 0; i < sprites_count; i++) {
        Sprite *sprite_i = &sprites_array[i];
        Vec2 player_to_sprite_i = vec2_subtract_vec2_cp(&sprite_i->position, &camera->position);
        double player_sprite_distance_i = vec2_get_length(&player_to_sprite_i);

        for (int j = i + 1; j < sprites_count; j++) {
            Sprite *sprite_j = &sprites_array[j];

            Vec2 player_to_sprite_j = vec2_subtract_vec2_cp(&sprite_j->position, &camera->position);
            double player_sprite_distance_j = vec2_get_length(&player_to_sprite_j);

            if (player_sprite_distance_i < player_sprite_distance_j) {
                Sprite tmp = *sprite_j;

                sprites_array[j] = *sprite_i;
                sprites_array[i] = tmp;

                sprite_i = &sprites_array[i];
            }
        }
    }
}

void sprite_render(const Sprite *sprite_to_render, const Camera *camera, Window *window, int map_width, int map_height, const double *z_buffer) {
    int window_width = window_get_width(window);
    int window_height = window_get_height(window);
    int half_window_height = window_height / 2.0;

    Vec2 map_sprite_pos = vec2_map_norm_coord_cp(&sprite_to_render->position, map_width, map_height);
    Vec2 map_player_pos = vec2_map_norm_coord_cp(&camera->position, map_width, map_height);

    Vec2 player_to_sprite = vec2_subtract_vec2_cp(&map_sprite_pos, &map_player_pos);

    double player_sprite_distance = vec2_get_length(&player_to_sprite);

    if (player_sprite_distance > 0.0) {

        double player_sprite_angle = atan2(player_to_sprite.y, player_to_sprite.x);
        player_sprite_angle = RADS_TO_DEG(player_sprite_angle);

        if (player_sprite_angle > 360.0)
            player_sprite_angle -= 360.0;
        else if (player_sprite_angle < 0.0)
            player_sprite_angle += 360.0;

        // calculate the column where the sprite resides
        double sprite_column = camera->look_at + (camera->fov / 2.0) - player_sprite_angle;

        if (camera->look_at >= 0.0 && camera->look_at <= 90.0 && player_sprite_angle >= 270.0 && player_sprite_angle <= 360.0)
            sprite_column += 360;
        else if (camera->look_at >= 270.0 && camera->look_at <= 360.0 && player_sprite_angle >= 0.0 && player_sprite_angle <= 90.0)
            sprite_column -= 360;

        // correcting distance
        player_sprite_distance *= cos(DEG_TO_RADS(player_sprite_angle - camera->look_at));

        // TODO: find a better way to clip sprites which are at 90 or -90 degrees from the camera
        if (player_sprite_distance < 0.4)
            return;

        Vec2 sprite_screen_pos = {
            .x = window_width - sprite_column * (window_width / camera->fov),
            .y = (window_height / 2.0)
        };

        int sprite_height = window_height / player_sprite_distance;
        int sprite_width = window_width / player_sprite_distance;

        int half_sprite_height = sprite_height / 2.0;
        int half_sprite_width = sprite_width / 2.0;

        int sprite_top = half_window_height - half_sprite_height;
        int sprite_bottom = half_window_height + half_sprite_height;
        int sprite_left = sprite_screen_pos.x - half_sprite_width;
        int sprite_right = sprite_screen_pos.x + half_sprite_width;

        int sprite_in_view_top = sprite_top < 0 ? 0 : sprite_top;
        int sprite_in_view_bottom = sprite_bottom > window_height ? window_height : sprite_bottom;
        int sprite_in_view_left = sprite_left < 0 ? 0 : sprite_left;
        int sprite_in_view_right = sprite_right > window_width ? window_width : sprite_right;

        int sprite_in_view_height = sprite_in_view_bottom - sprite_in_view_top;

        int sprite_in_view_x = sprite_left < 0 ? abs(sprite_left) : 0;
        int sprite_in_view_y = sprite_top < 0 ? abs(sprite_top) : 0;

        double sprite_height_texture_height_ratio = (double)sprite_to_render->texture->height / sprite_height;
        double sprite_width_texture_width_ratio = (double)sprite_to_render->texture->width / sprite_width;

        for (int x = sprite_in_view_left; x < sprite_in_view_right; x++, sprite_in_view_x++) {
            if (z_buffer[x] < player_sprite_distance)
                continue;

            RGBA line_colors[sprite_in_view_height] = {};
            int tex_x = sprite_in_view_x * sprite_width_texture_width_ratio;

            for (int y = sprite_in_view_top, sprite_y = sprite_in_view_y; y < sprite_in_view_bottom; y++, sprite_y++) {
                int tex_y = sprite_height_texture_height_ratio * sprite_y;

                line_colors[y - sprite_in_view_top] = texture_get_pixel(sprite_to_render->texture, tex_x, tex_y);
            }

            Vec2 line_start = {
                .x = (double)x / window_width,
                .y = (double)sprite_top / window_height,
            };
            Vec2 line_end = {
                .x = (double)x / window_width,
                .y = (double)sprite_bottom / window_height,
            };

            window_draw_line(window, line_start, line_end, line_colors, sprite_in_view_height);
        }
    }
}

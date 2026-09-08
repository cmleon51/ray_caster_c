#ifndef WINDOW_H
#define WINDOW_H

#include <linear_algebra/vec2.h>

typedef struct Window Window;

typedef struct {
    int r;
    int g;
    int b;
    int a;
} RGBA;

typedef enum {
    EVENT_NONE,
    EVENT_QUIT_APP,
    EVENT_WINDOW_RESIZED
} Event;

typedef enum {
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
} KeyPress;

Window *window_create();

void window_flip(Window *window);

void window_clear_surface(Window *window);

int window_resize(Window *window);

int window_should_quit(Window *window);

int window_get_width(Window *window);

int window_get_height(Window *window);

Event window_poll_event(Window *window);

int window_is_key_pressed(KeyPress key);

void window_draw_line(Window *window, Vec2 norm_start, Vec2 norm_end, RGBA *colors, int colors_count);

void window_close(Window *window);

void window_destroy(Window *window);

#endif // WINDOW_H

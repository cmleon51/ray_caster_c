#ifndef CAMERA_H
#define CAMERA_H

#include <linear_algebra/vec2.h>

typedef struct {
    Vec2 position;
    double look_at;
    double fov;
} Camera;

#endif // CAMERA_H

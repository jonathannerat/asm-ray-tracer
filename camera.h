#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"

typedef struct {
  point origin;
  point bl_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
  float lens_radius;
} camera;

camera camera_init(point from, point to, vec3 vup, float vfov,
                   float aspect_ratio, float aperture, float focus_dist);
#endif // CAMERA_H

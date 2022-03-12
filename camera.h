#ifndef CAMERA_H
#define CAMERA_H

#include "ray.h"

typedef struct {
  point origin;
  point bl_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
  double lens_radius;
} camera;

camera camera_init(point from, point to, vec3 vup, double vfov,
                   double aspect_ratio, double aperture, double focus_dist);
ray camera_get_ray(const camera *c, double s, double t);
#endif // CAMERA_H

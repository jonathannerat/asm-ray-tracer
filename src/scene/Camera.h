#ifndef CAMERA_H
#define CAMERA_H

#include "../vec3.h"

typedef struct {
  Point origin;
  Point bl_corner;
  Vec3 horizontal;
  Vec3 vertical;
  Vec3 u, v, w;
  real lens_radius;
} Camera;

Camera camera_new(Point, Point, Vec3, real, real, real, real);
Ray camera_get_ray(const Camera *, real, real);

#endif /* end of include guard: CAMERA_H */

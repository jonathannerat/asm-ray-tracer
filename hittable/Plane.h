#ifndef HITTABLE_PLANE_H
#define HITTABLE_PLANE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point origin;
  vec3 normal;
  Material *mat;
} Plane;

Plane *plane_init(point origin, vec3 normal, Material *m);

void plane_destroy(Plane *self);

#endif // HITTABLE_PLANE_H

#ifndef HITTABLE_PLANE_H
#define HITTABLE_PLANE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point origin;
  vec3 normal;
} Plane;

Plane *plane_init(point origin, vec3 normal);

void plane_destroy(Plane *self);

#endif // HITTABLE_PLANE_H

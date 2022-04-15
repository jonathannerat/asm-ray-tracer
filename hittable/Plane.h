#ifndef HITTABLE_PLANE_H
#define HITTABLE_PLANE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point origin;
  vec3 normal;
  Material *mat;
} Plane;

Hittable *plane_init(point origin, vec3 normal, Material *m);

#endif // HITTABLE_PLANE_H

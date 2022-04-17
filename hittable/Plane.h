#ifndef HITTABLE_PLANE_H
#define HITTABLE_PLANE_H

#include "../Hittable.h"
#include "../Material.h"

typedef struct {
  Hittable _hittable;
  point origin;
  vec3 normal;
  spmat *sm;
} Plane;

Hittable *plane_init(point origin, vec3 normal, spmat *sm);

#endif // HITTABLE_PLANE_H

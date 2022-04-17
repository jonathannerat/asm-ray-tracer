#ifndef SPHERE_H
#define SPHERE_H

#include "../Hittable.h"
#include "Box.h"

typedef struct {
  Hittable _hittable;
  point center;
  double radius;
  Box *bbox;
  spmat *sm;
} Sphere;

Hittable *sphere_init(point center, double radius, spmat *sm);

#endif // SPHERE_H

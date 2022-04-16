#ifndef SPHERE_H
#define SPHERE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point center;
  double radius;
  shrmat sm;
} Sphere;

Hittable *sphere_init(point center, double radius, shrmat sm);

#endif // SPHERE_H

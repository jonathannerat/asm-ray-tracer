#ifndef SPHERE_H
#define SPHERE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point center;
  double radius;
  Material *mat;
} Sphere;

Hittable *sphere_init(point center, double radius, Material *m);

#endif // SPHERE_H

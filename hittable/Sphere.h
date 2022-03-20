#ifndef SPHERE_H
#define SPHERE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point center;
  double radius;
} Sphere;

Sphere *sphere_init(point center, double radius);
void sphere_destroy(Sphere *sphere);

#endif // SPHERE_H

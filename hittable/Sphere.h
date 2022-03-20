#ifndef SPHERE_H
#define SPHERE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point center;
  double radius;
  Material *mat;
} Sphere;

Sphere *sphere_init(point center, double radius, Material *m);
void sphere_destroy(Sphere *sphere);

#endif // SPHERE_H

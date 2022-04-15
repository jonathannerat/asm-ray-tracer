#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  point p1, p2, p3;
  Material *mat;
} Triangle;

Hittable *triangle_init(point p1, point p2, point p3, Material *m);

#endif // TRIANGLE_H

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "../Hittable.h"
#include "Box.h"

typedef struct {
  Hittable _hittable;
  point p1, p2, p3;
  Box *bbox;
  shrmat sm;
} Triangle;

Hittable *triangle_init(point p1, point p2, point p3, shrmat sm);

#endif // TRIANGLE_H

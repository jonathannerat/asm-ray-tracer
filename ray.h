#ifndef RAY_H
#define RAY_H

#include <stdio.h>

#include "vec3.h"

typedef struct {
  point origin;
  vec3 direction;
} ray;

inline point ray_at(const ray *r, float t) {
  return vec3_add(r->origin, vec3_scale(t, r->direction));
}

#endif // RAY_H

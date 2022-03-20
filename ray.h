#ifndef RAY_H
#define RAY_H

#include <stdio.h>

#include "vec3.h"

#define RAY_BUF_SIZE 100

typedef struct {
  point origin;
  vec3 direction;
} ray;

inline point ray_at(const ray *r, float t) {
  return vec3_add(r->origin, vec3_scale(t, r->direction));
}

const char* ray_tostr(const ray *r);

#endif // RAY_H

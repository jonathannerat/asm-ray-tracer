#ifndef RAY_H
#define RAY_H

#include "vec3.h"

typedef struct {
  point o;
  vec3 d;
} ray;

inline point ray_at(const ray r, float t) {
  return vec3_add(r.o, vec3_scale(t, r.d));
}

#endif // RAY_H

#ifndef RECORD_H
#define RECORD_H

#include "ray.h"

typedef struct {
  double t;
  point p;
  vec3 normal;
  bool_ front_face;
} Record;

inline void hr_set_face_normal(Record *hr, const ray *r, vec3 n) {
  hr->front_face = dot(r->direction, n) < 0;
  hr->normal = hr->front_face ? n : vec3_inv(n);
}

#endif // RECORD_H

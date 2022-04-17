#ifndef RECORD_H
#define RECORD_H

#include "ray.h"

struct _material;
struct _spmat;

struct _record {
  double t;
  point p;
  vec3 normal;
  bool front_face;
  struct _spmat *sm;
};

typedef struct _record Record;

inline void hr_set_face_normal(Record *hr, const ray *r, vec3 n) {
  hr->front_face = dot(r->direction, n) < 0;
  hr->normal = hr->front_face ? n : vec3_inv(n);
}

#endif // RECORD_H

#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"

typedef struct {
  double t;
  point p;
  vec3 normal;
  bool_ front_face;
} HitRecord;

inline void hr_set_face_normal(HitRecord *hr, const ray *r, vec3 n) {
  hr->front_face = dot(r->direction, n) < 0;
  hr->normal = hr->front_face ? n : vec3_inv(n);
}

typedef struct _hittable Hittable;
typedef bool_ (*hit_method)(const Hittable *o, const ray *r, double t_min,
                            double t_max, HitRecord *hr);

struct _hittable {
  hit_method hit;
};

#endif // HITTABLE_H

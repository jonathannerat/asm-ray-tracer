#ifndef TRACER_H
#define TRACER_H

#include "materials.h"
#include "scene/Scene.h"

typedef struct {
  Point p;
  Vec3 normal;
  Material *m;
  real t;
  bool front_face;
} Record;

void tracer(Scene *);

Vec3 vec3_add(const Vec3 a, const Vec3 b);
Vec3 vec3_sub(const Vec3 a, const Vec3 b);
Vec3 vec3_prod(const Vec3 a, const Vec3 b);
Vec3 vec3_scale(const Vec3 a, real);
Vec3 vec3_unscale(const Vec3 a, real);
Vec3 vec3_inv(const Vec3 v);
real vec3_norm2(const Vec3 v);
real vec3_dot(const Vec3 a, const Vec3 b);
Vec3 vec3_to_normalized(const Vec3 v);
Vec3 vec3_cross(const Vec3 a, const Vec3 b);
Vec3 vec3_rnd_unit_sphere();
bool vec3_le(const Vec3, const Vec3);

#endif /* end of include guard: TRACER_H */

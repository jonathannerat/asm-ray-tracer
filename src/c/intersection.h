#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "../surfaces.h"
#include "../materials.h"

typedef struct {
  Point p;
  Vec3 normal;
  Material *m;
  real t;
  bool front_face;
} HitRecord;

bool list_hit(Surface *, const Ray *, real, real, HitRecord*);

bool plane_hit(Surface *, const Ray *, real, real, HitRecord*);

bool sphere_hit(Surface *, const Ray *, real, real, HitRecord*);

bool aabox_hit(Surface *, const Ray *, real, real, HitRecord*);

bool triangle_hit(Surface *, const Ray *, real, real, HitRecord*);

bool kdtree_hit(Surface *, const Ray *, real, real, HitRecord*);

#endif /* end of include guard: INTERSECTION_H */

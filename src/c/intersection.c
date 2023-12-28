#include "intersection.h"
#include "../structures/array.h"
#include "../vec3.h"

#include <math.h>
#include <stddef.h>

#define IS_PERPENDICULAR(a, b) (fabs(vec3_dot(a, b)) < EPS)

#define HIT_SURFACES(s, f)                                                               \
  t = s;                                                                                 \
  for (uint j = 0; j < self->count_by_type[t]; i++, j++) {                               \
    if (f(surfaces[i], r, t_min, closest_hit_so_far, &last_hit)) {                       \
      hit_anything = true;                                                               \
      closest_hit_so_far = last_hit.t;                                                   \
      *hit = last_hit;                                                                   \
    }                                                                                    \
  }

bool list_hit(Surface *s, const Ray *r, real t_min, real t_max, HitRecord *hit) {
  List *self = (List *)s;
  Surface **surfaces = self->surfaces;
  bool hit_anything = false;
  real closest_hit_so_far = t_max;
  HitRecord last_hit;

  enum surface_type t;
  uint i = 0;

  HIT_SURFACES(PLANE, plane_hit)
  HIT_SURFACES(SPHERE, sphere_hit)
  HIT_SURFACES(LIST, list_hit)
  HIT_SURFACES(AABOX, aabox_hit)
  HIT_SURFACES(TRIANGLE, triangle_hit)
  HIT_SURFACES(KDTREE, kdtree_hit)

  return hit_anything;
}

void save_hit(HitRecord *hit, const Ray *r, real t, const Vec3 n, Material *m) {
  bool is_front_face = vec3_dot(r->direction, n) < 0;
  hit->t = t;
  hit->p = ray_at(r, t);
  hit->m = m;
  hit->front_face = is_front_face;
  hit->normal = is_front_face ? n : vec3_inv(n);
}

bool plane_hit(Surface *s, const Ray *r, real t_min, real t_max, HitRecord *hit) {
  Plane *self = (Plane *)s;

  if (IS_PERPENDICULAR(r->direction, self->normal)) {
    // Surface is parallel to ray direction ==> it won't hit unless overlap occurs
    return false;
  }

  real t = vec3_dot(vec3_sub(self->origin, r->origin), self->normal) /
           vec3_dot(r->direction, self->normal);

  if (t < t_min || t_max < t)
    return false;

  save_hit(hit, r, t, self->normal, self->material);

  return true;
}

bool sphere_hit(Surface *s, const Ray *r, real t_min, real t_max, HitRecord *hit) {
  Sphere *self = (Sphere *)s;
  Vec3 oc = vec3_sub(r->origin, self->center);
  real a = vec3_norm2(r->direction);
  real hb = vec3_dot(oc, r->direction);
  real c = vec3_norm2(oc) - self->radius * self->radius;
  real discriminant = hb * hb - a * c;

  if (discriminant < 0)
    return false;

  real sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  real root = (-hb - sqrtd) / a;
  if (root < t_min || t_max < root) {
    root = (-hb + sqrtd) / a;
    if (root < t_min || t_max < root)
      return false;
  }

  Vec3 outward_normal =
    vec3_unscale(vec3_sub(ray_at(r, root), self->center), self->radius);
  save_hit(hit, r, root, outward_normal, self->material);

  return true;
}

bool aabox_contains(const AABox *self, Point p) {
  return vec3_le(vec3_sub(self->pmin, V3(EPS)), p) &&
         vec3_le(p, vec3_add(self->pmax, V3(EPS)));
}

bool aabox_hit(Surface *s, const Ray *r, real t_min, real t_max, HitRecord *hit) {
  AABox *self = (AABox *)s;
  bool hit_anything = false;
  real closest_so_far = t_max;
  HitRecord tmp;
  Plane **sides = self->sides;

  for (uint i = 0; i < arr_len(sides); i++) {
    if (plane_hit((Surface *)sides[i], r, t_min, closest_so_far, &tmp) &&
        aabox_contains(self, tmp.p)) {
      hit_anything = true;
      closest_so_far = tmp.t;
      *hit = tmp;
    }
  }

  return hit_anything;
}

// Point p is inside triangle for side (p2-p1)
#define IS_LEFT_OF(p, p1, p2)                                                            \
  (vec3_dot(normal, vec3_cross(vec3_sub(p2, p1), vec3_sub(p, p1))) > 0)

bool triangle_hit(Surface *s, const Ray *r, real t_min, real t_max, HitRecord *hit) {
  Triangle *self = (Triangle *)s;

  Vec3 normal = vec3_to_normalized(vec3_cross(vec3_sub(self->p2, self->p1), vec3_sub(self->p3, self->p1)));

  if (IS_PERPENDICULAR(normal, r->direction))
    return false;

  real t =
    vec3_dot(vec3_sub(self->p1, r->origin), normal) / vec3_dot(r->direction, normal);

  if (t < t_min || t_max < t)
    return false;

  Point p = ray_at(r, t);

  if (IS_LEFT_OF(p, self->p1, self->p2) && IS_LEFT_OF(p, self->p2, self->p3) &&
      IS_LEFT_OF(p, self->p3, self->p1)) {
    save_hit(hit, r, t, normal, self->material);
    return true;
  }

  return false;
}

bool kdtree_node_hit(Surface *s, const Ray *r, real t_min, real t_max, HitRecord *hit) {
  KDTreeNode *self = (KDTreeNode *)s;
  List *objects = self->objects;
  HitRecord tmp;

  if (!aabox_hit((Surface *)s->bounding_box, r, t_min, t_max, &tmp))
    return false;

  if (objects) {
    return list_hit((Surface *)objects, r, t_min, t_max, hit);
  } else {
    HitRecord left_rec, right_rec;
    bool left_hit = false, right_hit = false;

    left_hit = kdtree_node_hit((Surface *)self->left, r, t_min, t_max, &left_rec);
    right_hit = kdtree_node_hit((Surface *)self->right, r, t_min, t_max, &right_rec);

    if (left_hit && right_hit)
      *hit = left_rec.t < right_rec.t ? left_rec : right_rec;
    else if (left_hit)
      *hit = left_rec;
    else if (right_hit)
      *hit = right_rec;

    return left_hit || right_hit;
  }
}

bool kdtree_hit(Surface *s, const Ray *r, real t_min, real t_max, HitRecord *hit) {
  return kdtree_node_hit((Surface *)((KDTree *)s)->root, r, t_min, t_max, hit);
}

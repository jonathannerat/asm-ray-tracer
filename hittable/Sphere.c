#include "Sphere.h"

bool_ sphere_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);

Sphere *sphere_init(point center, double radius, Material *m) {
  Sphere *s = malloc(sizeof(Sphere));

  s->_hittable.hit = sphere_hit;

  s->center = center;
  s->radius = radius;

  s->mat = m;

  return s;
}

bool_ sphere_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
  Sphere *self = (Sphere *)_self;

  vec3 oc = vec3_sub(r->origin, self->center);
  double a = vec3_norm2(r->direction);
  double hb = dot(oc, r->direction);
  double c = vec3_norm2(oc) - self->radius * self->radius;
  double discriminant = hb * hb - a * c;

  if (discriminant < 0)
    return false;

  double sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  double root = (-hb - sqrtd) / a;
  if (root < t_min || t_max < root) {
    root = (-hb + sqrtd) / a;
    if (root < t_min || t_max < root)
      return false;
  }

  hr->t = root;
  hr->p = ray_at(r, hr->t);
  hr->mat = self->mat;
  vec3 outward_normal = vec3_scale(1 / self->radius, vec3_sub(hr->p, self->center));
  hr_set_face_normal(hr, r, outward_normal);

  return true;
}

void sphere_destroy(Sphere *sphere) { free(sphere); }

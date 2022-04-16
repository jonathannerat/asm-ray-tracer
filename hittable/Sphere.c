#include "Sphere.h"

bool sphere_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);
void sphere_destroy(Hittable *h);
Box *sphere_bbox(const Hittable *h);

Hittable *sphere_init(point center, double radius, shrmat sm) {
  Sphere *s = malloc(sizeof(Sphere));

  s->_hittable.hit = sphere_hit;
  s->_hittable.destroy = sphere_destroy;
  s->_hittable.bbox = sphere_bbox;

  s->center = center;
  s->radius = radius;

  vec3 r = {radius, radius, radius};
  s->bbox = (Box *)box_init(vec3_sub(center, r), vec3_add(center, r), (shrmat){.m = NULL});

  sm.refcount++;
  s->sm = sm;

  return (Hittable *)s;
}

bool sphere_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
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
  hr->mat = self->sm.m;
  vec3 outward_normal = vec3_scale(1 / self->radius, vec3_sub(hr->p, self->center));
  hr_set_face_normal(hr, r, outward_normal);

  return true;
}

void sphere_destroy(Hittable *h) {
  Sphere *self = (Sphere *)h;

  if (!--self->sm.refcount && self->sm.m)
    free(self->sm.m);

  DESTROY(self->bbox);

  free(h);
}

Box *sphere_bbox(const Hittable *h) { return ((Sphere *)h)->bbox; }

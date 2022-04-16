#include "Plane.h"

bool plane_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);
void plane_destroy(Hittable *h);

Hittable *plane_init(point origin, vec3 normal, shrmat sm) {
  Plane *p = malloc(sizeof(Plane));

  p->_hittable.hit = plane_hit;
  p->_hittable.destroy = plane_destroy;

  p->origin = origin;
  p->normal = normal;

  p->sm = sm;

  return (Hittable *)p;
}

bool plane_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
  Plane *self = (Plane *)_self;

  if (perpendicular(r->direction, self->normal))
    return false;

  double t = dot(vec3_sub(self->origin, r->origin), self->normal) / dot(r->direction, self->normal);

  if (t < t_min || t_max < t)
    return false;

  hr->t = t;
  hr->p = ray_at(r, t);
  hr->mat = self->sm.m;
  hr_set_face_normal(hr, r, self->normal);

  return true;
}

void plane_destroy(Hittable *h) {
  Plane *self = (Plane *)h;

  if (self->sm.refcount-- == 1)
    free(self->sm.m);

  free(h);
}

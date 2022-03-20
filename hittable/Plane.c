#include "Plane.h"

bool_ plane_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);

Plane *plane_init(point origin, vec3 normal, Material *m) {
  Plane *p = malloc(sizeof(Plane));

  p->_hittable.hit = plane_hit;

  p->origin = origin;
  p->normal = normal;

  p->mat = m;

  return p;
}

void plane_destroy(Plane *self) { free(self); }

bool_ plane_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
  Plane *self = (Plane *)_self;

  if (perpendicular(r->direction, self->normal))
    return false;

  double t = dot(vec3_sub(self->origin, r->origin), self->normal) / dot(r->direction, self->normal);

  if (t < t_min || t_max < t)
    return false;

  hr->t = t;
  hr->p = ray_at(r, t);
  hr->mat = self->mat;
  hr_set_face_normal(hr, r, self->normal);

  return true;
}

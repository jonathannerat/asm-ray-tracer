#include "Plane.h"
#include "Box.h"

bool plane_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr);
void plane_destroy(Hittable *h);
Box *plane_bbox(const Hittable *h) { return NULL; }

Hittable *plane_init(point origin, vec3 normal, spmat *sm) {
  Plane *p = malloc(sizeof(Plane));

  p->_hittable = (Hittable){
    plane_hit,
    plane_destroy,
    plane_bbox,
    origin,
  };

  p->origin = origin;
  p->normal = normal;

  if (sm)
    sm->c++;
  p->sm = sm;

  return (Hittable *)p;
}

bool plane_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr) {
  Plane *self = (Plane *)_self;

  if (perpendicular(r->direction, self->normal))
    return false;

  double t = dot(vec3_sub(self->origin, r->origin), self->normal) / dot(r->direction, self->normal);

  if (t < t_min || t_max < t)
    return false;

  hr->t = t;
  hr->p = ray_at(r, t);
  hr->sm = self->sm;
  hr_set_face_normal(hr, r, self->normal);

  return true;
}

void plane_destroy(Hittable *h) {
  Plane *self = (Plane *)h;

  spmat *sm = self->sm;
  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  free(h);
}

#include <stdlib.h>

#include "Sphere.h"
#include "Box.h"

bool sphere_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr);
void sphere_destroy(Hittable *h);
Box *sphere_bbox(const Hittable *h);

Hittable *sphere_init(point center, double radius, spmat *sm) {
  Sphere *s = malloc(sizeof(Sphere));

  s->_hittable = (Hittable){
    sphere_hit,
    sphere_destroy,
    sphere_bbox,
    center,
  };

  s->center = center;
  s->radius = radius;

  vec3 r = {radius, radius, radius};
  s->bbox = (Box *)box_init(vec3_sub(center, r), vec3_add(center, r), NULL);

  sm->c++;
  s->sm = sm;

  return (Hittable *)s;
}

bool sphere_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr) {
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
  hr->sm = self->sm;
  vec3 outward_normal = vec3_scale(1 / self->radius, vec3_sub(hr->p, self->center));
  hr_set_face_normal(hr, r, outward_normal);

  return true;
}

void sphere_destroy(Hittable *h) {
  Sphere *self = (Sphere *)h;

  spmat *sm = self->sm;
  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  DESTROY(self->bbox);

  free(h);
}

Box *sphere_bbox(const Hittable *h) { return ((Sphere *)h)->bbox; }

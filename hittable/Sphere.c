#include <stdlib.h>

#include "Sphere.h"
#include "Box.h"

void sphere_destroy(Hittable *h);
Box *sphere_bbox(const Hittable *h);

Hittable *sphere_init(Point center, double radius, spmat *sm) {
  Sphere *s = malloc(sizeof(Sphere));

  s->_hittable = (Hittable){
    sphere_hit,
    sphere_destroy,
    sphere_bbox,
    center,
  };

  s->center = center;
  s->radius = radius;

  Vec3 r = {radius, radius, radius};
  s->bbox = (Box *)box_init(vec3_sub(center, r), vec3_add(center, r), NULL);

  sm->c++;
  s->sm = sm;

  return (Hittable *)s;
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

#include <stdlib.h>

#include "Box.h"
#include "Sphere.h"

void sphere_destroy(Hittable *h);
Box *sphere_bbox(const Hittable *h);

Hittable *sphere_init(Point center, real radius, spmat *sm) {
  Sphere *s = malloc(sizeof(Sphere));

  s->_hittable = (Hittable){
    sphere_hit,
    sphere_destroy,
    sphere_bbox,
    center,
  };

  s->center = center;
  s->radius = radius;

  Vec3 cback = {center.x - radius, center.y - radius, center.z - radius};
  Vec3 cfront = {center.x + radius, center.y + radius, center.z + radius};
  s->bbox = (Box *)box_init(cback, cfront, NULL);

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

#include "Plane.h"
#include "Box.h"

Box *plane_bbox(const Hittable *h) { return NULL; }

Hittable *plane_init(Point origin, Vec3 normal, spmat *sm) {
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

void plane_destroy(Hittable *h) {
  Plane *self = (Plane *)h;

  spmat *sm = self->sm;
  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  free(h);
}

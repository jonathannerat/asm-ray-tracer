#include <stdlib.h>

#include "Triangle.h"
#include "Box.h"

void triangle_destroy(Hittable *h);
Box *triangle_bbox(const Hittable *h);

Hittable *triangle_init(Point p1, Point p2, Point p3, spmat *sm) {
  Triangle *self = malloc(sizeof(Triangle));
  Point refp = vec3_unscale(vec3_add(vec3_add(p1, p2), p3), 3);

  self->_hittable = (Hittable){
    triangle_hit,
    triangle_destroy,
    triangle_bbox,
    refp,
  };

  self->p1 = p1;
  self->p2 = p2;
  self->p3 = p3;

  Point cback = {MIN(MIN(p1.x, p2.x), p3.x), MIN(MIN(p1.y, p2.y), p3.y),
                 MIN(MIN(p1.z, p2.z), p3.z)};
  Point cfront = {MAX(MAX(p1.x, p2.x), p3.x), MAX(MAX(p1.y, p2.y), p3.y),
                  MAX(MAX(p1.z, p2.z), p3.z)};

  self->bbox = (Box *)box_init(cback, cfront, NULL);

  sm->c++;
  self->sm = sm;

  return (Hittable *)self;
}

void triangle_destroy(Hittable *h) {
  Triangle *self = (Triangle *)h;

  spmat *sm = self->sm;
  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  DESTROY(self->bbox);

  free(h);
}

Box *triangle_bbox(const Hittable *h) { return ((Triangle *)h)->bbox; }

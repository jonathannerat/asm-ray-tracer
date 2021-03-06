#include "Box.h"
#include "List.h"
#include "Plane.h"

void box_destroy(Hittable *box);
Box *box_bbox(const Hittable *h);

/** Checks if the point p is inside box
 *
 * @param box box boundary
 * @param p point to check
 * @return true iff it's inside
 */
bool box_is_inside(const Box *self, Point p) {
  return self->cback.x - EPS <= p.x && p.x <= self->cfront.x + EPS &&
         self->cback.y - EPS <= p.y && p.y <= self->cfront.y + EPS &&
         self->cback.z - EPS <= p.z && p.z <= self->cfront.z + EPS;
}

Hittable *box_init(Point p1, Point p2, spmat *sm) {
  Box *b = malloc(sizeof(Box));

  b->_hittable = (Hittable){
    box_hit,
    box_destroy,
    box_bbox,
  };

  b->cback = (Point){
    MIN(p1.x, p2.x),
    MIN(p1.y, p2.y),
    MIN(p1.z, p2.z),
  };

  b->cfront = (Point){
    MAX(p1.x, p2.x),
    MAX(p1.y, p2.y),
    MAX(p1.z, p2.z),
  };

  b->_hittable.refp = vec3_unscale(vec3_add(b->cback, b->cfront), 2);

  if (sm)
    sm->c++;
  b->sm = sm;

  b->faces = (List *)list_init();

  list_push(b->faces, plane_init(b->cback, V(-1,0,0), sm));
  list_push(b->faces, plane_init(b->cback, V(0,-1,0), sm));
  list_push(b->faces, plane_init(b->cback, V(0,0,-1), sm));
  list_push(b->faces, plane_init(b->cfront, V(1,0,0), sm));
  list_push(b->faces, plane_init(b->cfront, V(0,1,0), sm));
  list_push(b->faces, plane_init(b->cfront, V(0,0,1), sm));

  return (Hittable *)b;
}

void box_destroy(Hittable *h) {
  Box *self = (Box *)h;
  DESTROY(self->faces);

  spmat *sm = self->sm;
  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  free(self);
}

Box *box_bbox(const Hittable *h) { return (Box *)h; }

Box *box_join(const Box *a, const Box *b) {
  Point cb = a->cback, cf = a->cfront;
  Point bcb = b->cback, bcf = b->cfront;
  bool changed = false;

  if (!box_is_inside(a, bcb)) {
    cb.x = MIN(cb.x, bcb.x);
    cb.y = MIN(cb.y, bcb.y);
    cb.z = MIN(cb.z, bcb.z);
    changed = true;
  }

  if (!box_is_inside(a, bcf)) {
    cf.x = MAX(cf.x, bcf.x);
    cf.y = MAX(cf.y, bcf.y);
    cf.z = MAX(cf.z, bcf.z);
    changed = true;
  }

  Box *r = (Box *)a;

  if (changed) {
    r = (Box *)box_init(cb, cf, a->sm);
    DESTROY(a);
  }

  return r;
}

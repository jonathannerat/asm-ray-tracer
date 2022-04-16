#include "Box.h"

bool box_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);
void box_destroy(Hittable *box);
Box *box_bbox(const Hittable *h);

Hittable *box_init(point p1, point p2, shrmat sm) {
  Box *b = malloc(sizeof(Box));

  b->_hittable.hit = box_hit;
  b->_hittable.destroy = box_destroy;
  b->_hittable.bbox = box_bbox;

  b->cback = (point){
      MIN(p1.x, p2.x),
      MIN(p1.y, p2.y),
      MIN(p1.z, p2.z),
  };

  b->cfront = (point){
      MAX(p1.x, p2.x),
      MAX(p1.y, p2.y),
      MAX(p1.z, p2.z),
  };

  sm.refcount++;
  b->sm = sm;

  vec3 x = {1, 0, 0}, y = {0, 1, 0}, z = {0, 0, 1};

  b->faces = (List *)list_init();
  list_push(b->faces, plane_init(b->cback, vec3_inv(x), sm));
  list_push(b->faces, plane_init(b->cback, vec3_inv(y), sm));
  list_push(b->faces, plane_init(b->cback, vec3_inv(z), sm));
  list_push(b->faces, plane_init(b->cfront, x, sm));
  list_push(b->faces, plane_init(b->cfront, y, sm));
  list_push(b->faces, plane_init(b->cfront, z, sm));

  return (Hittable *)b;
}

bool box_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
  Box *self = (Box *)_self;
  size_t i;
  bool hit_anything = false;
  double closest_so_far = t_max;
  Record tmp;

  for (i = 0; i < self->faces->size; i++) {
    Hittable *h = list_get(self->faces, i);
    if (h->hit(h, r, t_min, closest_so_far, &tmp) && box_is_inside(self, tmp.p)) {
      hit_anything = true;
      closest_so_far = tmp.t;
      *hr = tmp;
    }
  }

  return hit_anything;
}

void box_destroy(Hittable *h) {
  Box *self = (Box *)h;
  DESTROY(self->faces);

  if (!--self->sm.refcount && self->sm.m)
    free(self->sm.m);

  free(self);
}

Box *box_bbox(const Hittable *h) { return (Box *)h; }

Box *box_join(const Box *a, const Box *b) {
  point cb = a->cback, cf = a->cfront;
  point bcb = b->cback, bcf = b->cfront;

  if (!box_is_inside(a, bcb)) {
    cb.x = MIN(cb.x, bcb.x);
    cb.y = MIN(cb.y, bcb.y);
    cb.z = MIN(cb.z, bcb.z);
  }

  if (!box_is_inside(a, bcf)) {
    cf.x = MAX(cf.x, bcf.x);
    cf.y = MAX(cf.y, bcf.y);
    cf.z = MAX(cf.z, bcf.z);
  }

  return (Box *)box_init(cb, cf, a->sm);
}

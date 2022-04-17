#include "Box.h"

bool box_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);
void box_destroy(Hittable *box);
Box *box_bbox(const Hittable *h);

Hittable *box_init(point p1, point p2, spmat *sm) {
  Box *b = malloc(sizeof(Box));

  b->_hittable = (Hittable){
    box_hit,
    box_destroy,
    box_bbox,
  };

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

  b->_hittable.refp = vec3_unscale(vec3_add(b->cback, b->cfront), 2);

  if (sm)
    sm->c++;
  b->sm = sm;

  vec3 x = {1, 0, 0}, y = {0, 1, 0}, z = {0, 0, 1};

  b->faces = (List *)list_init();
  list_push(b->faces, plane_init(b->cback, vec3_inv(x), NULL));
  list_push(b->faces, plane_init(b->cback, vec3_inv(y), NULL));
  list_push(b->faces, plane_init(b->cback, vec3_inv(z), NULL));
  list_push(b->faces, plane_init(b->cfront, x, NULL));
  list_push(b->faces, plane_init(b->cfront, y, NULL));
  list_push(b->faces, plane_init(b->cfront, z, NULL));

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

  spmat *sm = self->sm;
  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  free(self);
}

Box *box_bbox(const Hittable *h) { return (Box *)h; }

Box *box_join(const Box *a, const Box *b) {
  point cb = a->cback, cf = a->cfront;
  point bcb = b->cback, bcf = b->cfront;
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

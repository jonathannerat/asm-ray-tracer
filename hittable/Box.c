#include "Box.h"

bool box_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);
bool box_is_inside(Box *b, point p);
void box_destroy(Hittable *box);

Hittable *box_init(point p1, point p2, Material *m) {
  Box *b = malloc(sizeof(Box));

  b->_hittable.hit = box_hit;
  b->_hittable.destroy = box_destroy;

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

  b->mat = m;

  vec3 x = {1, 0, 0}, y = {0, 1, 0}, z = {0, 0, 1};

  b->faces = (List *)list_init();
  list_push(b->faces, plane_init(b->cback, vec3_inv(x), m));
  list_push(b->faces, plane_init(b->cback, vec3_inv(y), m));
  list_push(b->faces, plane_init(b->cback, vec3_inv(z), m));
  list_push(b->faces, plane_init(b->cfront, x, m));
  list_push(b->faces, plane_init(b->cfront, y, m));
  list_push(b->faces, plane_init(b->cfront, z, m));

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
  free(self->mat);
  free(self);
}

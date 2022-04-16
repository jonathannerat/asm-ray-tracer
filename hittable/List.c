#include "List.h"

#include "Box.h"

#define LIST_INITIAL_CAPACITY 16

bool list_hit(const Hittable *o, const ray *r, double t_min, double t_max, Record *hr);
void list_destroy(Hittable *self);
Box *list_bbox(const Hittable *h);

Hittable *list_init() {
  List *self = malloc(sizeof(List));

  self->_hittable.hit = list_hit;
  self->_hittable.destroy = list_destroy;
  self->_hittable.bbox = list_bbox;

  self->size = 0;
  self->cap = LIST_INITIAL_CAPACITY;
  self->list = malloc(sizeof(Hittable *) * self->cap);
  self->bbox = NULL;

  return (Hittable *)self;
}

bool list_push(List *self, Hittable *h) {
  if (self->size == self->cap) {
    self->cap *= 2;
    Hittable **list = realloc(self->list, sizeof(Hittable *) * self->cap);

    if (!list)
      return false;

    self->list = list;
  }

  self->list[self->size++] = h;

  Box *hbox = h->bbox(h);

  if (!self->bbox && hbox) {
    self->bbox = (Box *)box_init(hbox->cback, hbox->cfront, (shrmat){.m = NULL});
  } else if (hbox) {
    Box *newbbox = box_join(self->bbox, hbox);
    DESTROY(self->bbox);
    self->bbox = newbbox;
  }

  return true;
}

Hittable *list_get(List *l, size_t i) {
  if (i >= l->size)
    return NULL;

  return l->list[i];
}

bool list_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
  List *self = (List *)_self;
  Record tmp;
  u_int32_t i;
  bool hit_anything = false;
  double closest_so_far = t_max;

  for (i = 0; i < self->size; i++) {
    Hittable *h = self->list[i];
    if (h->hit(h, r, t_min, closest_so_far, &tmp)) {
      hit_anything = true;
      closest_so_far = tmp.t;
      *hr = tmp;
    }
  }

  return hit_anything;
}

void list_destroy(Hittable *h) {
  List *self = (List *)h;
  size_t i;

  for (i = 0; i < self->size; i++)
    DESTROY(self->list[i]);

  free(self->list);

  if (self->bbox)
    DESTROY(self->bbox);

  free(self);
}

Box *list_bbox(const Hittable *h) { return ((const List *)h)->bbox; }

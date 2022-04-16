#include "List.h"

#define LIST_INITIAL_CAPACITY 16

bool list_hit(const Hittable *o, const ray *r, double t_min, double t_max, Record *hr);
void list_destroy(Hittable *self);

Hittable *list_init() {
  List *self = malloc(sizeof(List));

  self->_hittable.hit = list_hit;
  self->_hittable.destroy = list_destroy;
  self->list = malloc(LIST_INITIAL_CAPACITY * sizeof(Hittable *));
  self->size = 0;
  self->cap = LIST_INITIAL_CAPACITY;

  return (Hittable *)self;
}

bool list_push(List *self, Hittable *h) {
  if (self->size == self->cap) {
    Hittable **list = realloc(self->list, sizeof(Hittable *) * self->cap * 2);
    if (!list)
      return false;

    if (list != self->list)
      self->list = list;
    self->cap *= 2;
  }

  self->list[self->size] = h;
  self->size++;

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
  DESTROY(*((List *)h)->list);
  free(h);
}

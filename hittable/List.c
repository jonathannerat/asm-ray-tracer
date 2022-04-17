#include "List.h"

#include "Box.h"
#include "Triangle.h"
#include "array.h"
#include <string.h>

#define LIST_INITIAL_CAPACITY 16

bool list_hit(const Hittable *o, const ray *r, double t_min, double t_max, Record *hr);
void list_destroy(Hittable *self);
Box *list_bbox(const Hittable *h);

Hittable *list_init() {
  List *self = malloc(sizeof(List));

  self->_hittable = (Hittable){list_hit, list_destroy, list_bbox, {NAN, NAN, NAN}};

  self->size = 0;
  self->cap = LIST_INITIAL_CAPACITY;
  self->list = malloc(sizeof(Hittable *) * self->cap);
  self->bbox = NULL;
  self->sm = NULL;
  self->refpsum = (point){0, 0, 0};

  return (Hittable *)self;
}

List *list_copy(List *l) {
  List *other = malloc(sizeof(List));
  size_t bytes = sizeof(Hittable *) * l->cap;

  other->size = l->size;
  other->cap = l->cap;
  other->list = malloc(bytes);
  other->bbox = l->bbox;
  other->_hittable = l->_hittable;

  if (l->sm)
    l->sm->c++;
  other->sm = l->sm;

  memcpy(other->list, l->list, bytes);

  return other;
}

void list_copy_destroy(List *l) {
  spmat *sm = l->sm;

  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  free(l->list);
  free(l);
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

  // update reference point
  self->refpsum = vec3_add(self->refpsum, h->refp);
  self->_hittable.refp = vec3_unscale(self->refpsum, self->size);

  Box *hbox = h->bbox(h);

  if (!self->bbox)
    self->bbox = hbox ? (Box *)box_init(hbox->cback, hbox->cfront, NULL) : NULL;
  else if (hbox)
    self->bbox = box_join(self->bbox, hbox);

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

  spmat *sm = self->sm;
  if (sm && !--sm->c && sm->m) {
    free(sm->m);
    free(sm);
  }

  if (self->bbox)
    DESTROY(self->bbox);

  free(self);
}

Box *list_bbox(const Hittable *h) { return ((const List *)h)->bbox; }

Hittable *list_parse_obj(const char *path, spmat *sm) {
  List *objects = (List *)list_init();

  if (sm)
    sm->c++;
  objects->sm = sm;

  char buf[MAX_BUF_SIZE];
  FILE *fp = fopen(path, "r");

  array_vec3 *vertex = array_vec3_init();

  while (fgets(buf, MAX_BUF_SIZE, fp)) {
    char *c = buf;

    switch (c[0]) {
    case 'v':
      if (c[1] == ' ') {
        c += 2;
        array_vec3_push(vertex, parse_vec3(c, NULL));
      }
      break;
    case 'f':
      if (c[1] == ' ') {
        c += 2; // skip to values
        size_t i, j, k;

        i = strtold(c, &c);
        c = strfind(c, ' ') + 1; // skip vn index
        j = strtold(c, &c);
        c = strfind(c, ' ') + 1;
        k = strtold(c, NULL);

        list_push(objects,
                  triangle_init(array_vec3_get(vertex, i - 1), array_vec3_get(vertex, j - 1),
                                array_vec3_get(vertex, k - 1), sm));
      }
      break;
    }
  }

  array_vec3_destroy(vertex);
  fclose(fp);

  return (Hittable *)objects;
}

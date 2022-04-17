#ifndef LIST_H
#define LIST_H

#include "../Hittable.h"
#include "../Material.h"

typedef struct {
  Hittable _hittable;
  Hittable **list;
  size_t size;
  size_t cap;
  point refpsum;
  spmat *sm;
  struct _box *bbox;
} List;

Hittable *list_init();

List *list_copy(List *l);

void list_copy_destroy(List *l);

Hittable *list_parse_obj(const char *, spmat *);

bool list_push(List *l, Hittable *h);

Hittable *list_get(List *l, size_t i);

#endif // LIST_H

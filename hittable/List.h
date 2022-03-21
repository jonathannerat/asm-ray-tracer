#ifndef LIST_H
#define LIST_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  Hittable **list;
  size_t size;
  size_t cap;
} List;

List *list_init();

bool list_push(List *l, Hittable *h);

Hittable *list_get(List *l, size_t i);

void list_destroy(List *l);

#endif // LIST_H

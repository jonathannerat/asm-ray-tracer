#ifndef LIST_H
#define LIST_H

#include "../Hittable.h"

typedef struct {
  Hittable _hittable;
  Hittable **list;
  u_int32_t size;
  u_int32_t cap;
} List;

List *list_init();

bool_ list_push(List *l, Hittable *h);

void list_destroy(List *l);

#endif // LIST_H

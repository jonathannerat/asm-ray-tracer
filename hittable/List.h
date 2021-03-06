#ifndef LIST_H
#define LIST_H

#include "core.h"

Hittable *list_init();

List *list_copy(List *l);

void list_copy_destroy(List *l);

#ifndef DEBUG_ASM
Hittable *list_parse_obj(const char *, spmat *);
#endif

bool list_push(List *l, Hittable *h);

Hittable *list_get(List *l, uint i);

#endif // LIST_H

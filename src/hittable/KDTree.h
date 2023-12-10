#ifndef KDTREE_H
#define KDTREE_H

#include "../core.h"

Hittable *kdtree_new(List *objects, uint leaf_size);
Hittable *kdtree_new_from_file(const char *path, uint leaf_size, spmat *sm);

#endif // KDTREE_H

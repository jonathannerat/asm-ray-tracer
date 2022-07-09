#ifndef KDTREE_H
#define KDTREE_H

#include "List.h"

struct _node {
  Hittable _hittable;
  struct _node *left, *right;
  struct _box *bbox;
  List *objects;
};

typedef struct _node Node;

typedef struct {
  Hittable _hittable;
  Node *root;
} KDTree;

Hittable *kdtree_init(List *objects, size_t leaf_size);
Hittable *kdtree_init_from_file(const char* path, size_t leaf_size, spmat *sm);

#endif // KDTREE_H

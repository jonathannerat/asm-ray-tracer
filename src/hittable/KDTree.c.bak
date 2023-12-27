#include <stdlib.h>
#include <string.h>

#include "KDTree.h"
#include "List.h"
#include "../util.h"

bool kdtree_node_hit(const Hittable *o, const Ray *r, real t_min, real t_max, Record *hr);
void kdtree_node_destroy(Hittable *o);
Box *kdtree_node_bbox(const Hittable *o) { return ((Node *)o)->bbox; }

Node *kdtree_node_new(List *objects) {
  Node *self = malloc(sizeof(Node));

  self->_hittable = (Hittable){
    kdtree_node_hit,
    kdtree_node_destroy,
    kdtree_node_bbox,
    objects->_hittable.refp,
  };

  self->left = NULL;
  self->right = NULL;
  self->objects = objects;
  self->bbox = objects->bbox;

  return self;
}

size_t kdtree_node_size(Node *n) {
  return n->objects ? n->objects->size : kdtree_node_size(n->left) + kdtree_node_size(n->right);
}

char axis = 'x';

int sort_objects_by_axis(const void *a, const void *b) {
  Hittable *ha = (Hittable *)a, *hb = (Hittable *)b;

  if (axis == 'x')
    return ha->refp.x < hb->refp.x ? -1 : 1;
  else if (axis == 'y')
    return ha->refp.y < hb->refp.y ? -1 : 1;
  else
    return ha->refp.z < hb->refp.z ? -1 : 1;
}

void kdtree_node_split(Node *n, size_t leaf_size) {
  if (!n->objects)
    return;

  // get axis with max diff
  Box *bbox = n->objects->bbox;
  Point cb = bbox->cback, cf = bbox->cfront;
  real maxdiff = cf.x - cb.x;
  axis = 'x';

  if (cf.y - cb.y > maxdiff) {
    maxdiff = cf.y - cb.y;
    axis = 'y';
  }

  if (cf.z - cb.z > maxdiff)
    axis = 'z';

  // order objects by that axis
  List *sorted = list_copy(n->objects);
  qsort(sorted->list, sorted->size, sizeof(Hittable *), sort_objects_by_axis);

  size_t middle = sorted->size / 2, i;
  List *hleft = (List *)list_init(), *hright = (List *)list_init();

  for (i = 0; i < middle; i++)
    list_push(hleft, list_get(sorted, i));

  for (i = middle; i < sorted->size; i++)
    list_push(hright, list_get(sorted, i));

  list_copy_destroy(sorted);

  n->left = kdtree_node_new(hleft);
  n->right = kdtree_node_new(hright);

  if (n->objects->size > leaf_size) {
    list_copy_destroy(n->objects);
    n->objects = NULL;
  }

  if (hleft->size > leaf_size)
    kdtree_node_split(n->left, leaf_size);
  if (hright->size > leaf_size)
    kdtree_node_split(n->right, leaf_size);
}

bool kdtree_node_hit(const Hittable *o, const Ray *r, real t_min, real t_max, Record *hr) {
  Box *bbox = o->bbox(o);
  Node *self = (Node *)o;
  List *objects = self->objects;
  Record tmp;

  if (!bbox || !HIT(bbox, r, t_min, t_max, &tmp))
    return false;

  if (objects) {
    return HIT(objects, r, t_min, t_max, hr);
  } else {
    Record lrec, rrec;
    bool lhit = false, rhit = false;

    if (self->left)
      lhit = HIT(self->left, r, t_min, t_max, &lrec);

    if (self->right)
      rhit = HIT(self->right, r, t_min, t_max, &rrec);

    if (lhit && rhit)
      *hr = lrec.t < rrec.t ? lrec : rrec;
    else if (lhit || rhit)
      *hr = lhit ? lrec : rrec;

    return lhit || rhit;
  }
}

void kdtree_node_destroy(Hittable *h) {
  Node *self = (Node *)h;

  if (self->left)
    DESTROY(self->left);

  if (self->right)
    DESTROY(self->right);

  if (self->objects)
    DESTROY(self->objects);
  else
    DESTROY(self->bbox);

  free(self);
}

bool kdtree_hit(const Hittable *o, const Ray *r, real t_min, real t_max, Record *hr) {
  return HIT(((KDTree *)o)->root, r, t_min, t_max, hr);
}

void kdtree_destroy(Hittable *o) {
  DESTROY(((KDTree *)o)->root);
  free(o);
}

Box *kdtree_bbox(const Hittable *o) { return ((KDTree *)o)->root->bbox; }

Hittable *kdtree_new(List *objects, uint leaf_size) {
  KDTree *self = malloc(sizeof(KDTree));

  self->_hittable = (Hittable){
    kdtree_hit,
    kdtree_destroy,
    kdtree_bbox,
    objects->_hittable.refp,
  };

  self->root = kdtree_node_new(objects);

  kdtree_node_split(self->root, leaf_size);

  return (Hittable *)self;
}

Hittable *kdtree_new_from_file(const char *path, uint leaf_size, spmat *sm) {
  return kdtree_new((List *)list_parse_obj(path, sm), leaf_size);
}

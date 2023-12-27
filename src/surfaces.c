#include "surfaces.h"
#include "assert.h"
#include "structures/array.h"
#include "vec3.h"

#include <stdio.h>
#include <stdlib.h>

AABox *aabox_add(AABox *, AABox *);

Plane *plane_new(Point origin, Vec3 normal, Material *material) {
  Plane *p = malloc(sizeof(Plane));

  *p = (Plane){
    {
      .type = PLANE, .bounding_box = NULL /* an infinite surface cannot be enclosed */
    },
    origin,
    vec3_to_normalized(normal),
    shpt_get(material)};

  return p;
}

void plane_free(Surface *p) {
  shpt_free(((Plane *)p)->material);
  free(p);
}

Sphere *sphere_new(Point center, real radius, Material *material) {
  Sphere *s = malloc(sizeof(Sphere));
  Vec3 radius_vec = V(radius, radius, radius);

  *s = (Sphere){{.type = SPHERE,
                 .bounding_box = aabox_new(vec3_sub(center, radius_vec),
                                           vec3_add(center, radius_vec), NULL),
                 .reference = center},
                center,
                radius,
                shpt_get(material)};

  return s;
}

void sphere_free(Surface *s) {
  shpt_free(((Sphere *)s)->material);
  aabox_free((Surface *)s->bounding_box);
  free(s);
}

int sort_surfaces_by_type(const void *a, const void *b) {
  Surface *sa = (Surface *)a, *sb = (Surface *)b;

  return sa->type - sb->type;
}

/**
 * @brief Create a bounding box for surfaces
 *
 * @param surfaces Array of surfaces to enclose
 * @return Smallest AABox that enclosest all surfaces
 */
AABox *bounding_box_for(Surface **surfaces) {
  AABox *result = surfaces[0]->bounding_box;

  // First surface might be a PLANE, which doesn't have a bbox
  if (!result)
    return NULL;

  result = aabox_new(result->pmin, result->pmax, NULL);
  uint len = arr_len(surfaces);

  for (uint i = 1; i < len; i++) {
    AABox *b = surfaces[i]->bounding_box;

    if (!b)
      return NULL;

    result = aabox_add(result, b);
  }

  return result;
}

/**
 * @param surfaces Array of Surface pointers, TAKES OWNERSHIP.
 * @return a Surface composed of other Surfaces
 */
List *list_new(Surface **surfaces) {
  // Surface list should have at least one element
  uint len = arr_len(surfaces);
  assert(len > 0);

  List *l = malloc(sizeof(List));
  AABox *b = bounding_box_for(surfaces);
  uint count = 0;
  enum surface_type type = PLANE;

  *l =
    (List){{.type = LIST, .bounding_box = b, .reference = b ? b->base.reference : V3(0)},
           surfaces,
           {0}};

  qsort(surfaces, len, sizeof(Surface *), sort_surfaces_by_type);

  for (uint i = 0; i < len; i++) {
    if (surfaces[i]->type != type) {
      l->count_by_type[type] = count;
      count = 0;
      while (surfaces[i]->type != type)
        type++;
    }
    count++;
  }

  l->count_by_type[type] = count;

  return l;
}

typedef void (*free_func)(Surface *self);

free_func free_functions[] = {
  [PLANE] = plane_free,   [SPHERE] = sphere_free,     [LIST] = list_free,
  [AARECT] = aarect_free, [AABOX] = aabox_free,       [TRIANGLE] = triangle_free,
  [KDTREE] = kdtree_free, [SURFACE_TYPE_SIZE] = NULL,
};

void list_free(Surface *s) {
  // Might not have bounding_box, since it can contain planes
  if (s->bounding_box)
    aabox_free((Surface *)s->bounding_box);

  Surface **surfaces = ((List *)s)->surfaces;
  enum surface_type last_type = SURFACE_TYPE_SIZE;
  free_func last_free_func = NULL;

  for (uint i = 0; i < arr_len(surfaces); i++) {
    if (surfaces[i]->type != last_type) {
      last_type = surfaces[i]->type;
      last_free_func = free_functions[last_type];
    }

    last_free_func(surfaces[i]);
  }

  arr_free(surfaces);
  free(s);
}

AARect *aarect_new(AARectParams params, Material *material) {
  AARect *r = malloc(sizeof(AARect));

  // Set pmin and pmax so that at the axis perpendicular to its plane, the width is EPS,
  // and at the remaining axis, the width is given by its size
  Point pmin, pmax;
  VEC_AT(pmin, params.plane) = params.x3 - EPS / 2;
  VEC_AT(pmin, (params.plane + 1) % 3) = params.x1_min;
  VEC_AT(pmin, (params.plane + 2) % 3) = params.x2_min;
  VEC_AT(pmax, params.plane) = params.x3 + EPS / 2;
  VEC_AT(pmax, (params.plane + 1) % 3) = params.x1_max;
  VEC_AT(pmax, (params.plane + 2) % 3) = params.x2_max;

  *r = (AARect){{.type = AARECT,
                 .bounding_box = aabox_new(pmin, pmax, NULL),
                 .reference = vec3_unscale(vec3_add(pmin, pmax), 2)},
                params,
                shpt_get(material)};

  return r;
}

void aarect_free(Surface *s) {
  shpt_free(((AARect *)s)->material);
  aabox_free((Surface *)s->bounding_box);
  free(s);
}

AABox *aabox_new(Point pmin, Point pmax, Material *m) {
  // p1 must have coordinates lower than those of p2
  assert(pmin.x < pmax.x && pmin.y < pmax.y && pmin.z < pmax.z);

  AABox *b = malloc(sizeof(AABox));
  Plane **sides = NULL;

  arr_grow(sides, 6);
  arr_push(sides, plane_new(pmin, V(-1, 0, 0), m));
  arr_push(sides, plane_new(pmin, V(0, -1, 0), m));
  arr_push(sides, plane_new(pmin, V(0, 0, -1), m));
  arr_push(sides, plane_new(pmax, V(1, 0, 0), m));
  arr_push(sides, plane_new(pmax, V(0, 1, 0), m));
  arr_push(sides, plane_new(pmax, V(0, 0, 1), m));

  *b = (AABox){{.type = AABOX, .bounding_box = b, .reference = b->base.reference},
               pmin,
               pmax,
               sides,
               shpt_get(m)};

  return b;
}

Vec3 aabox_sizes(const AABox *self) {
  Point pmin = self->pmin, pmax = self->pmax;
  return V(pmax.x - pmin.x, pmax.y - pmin.y, pmax.z - pmin.z);
}

/**
 * @brief Creates the smallest AABox that contains both a and b
 *
 * @param a first AABox, function TAKES OWNERSHIP of this parameter
 * @param b second AABox
 * @return AABox that contains both a and b
 */
AABox *aabox_add(AABox *a, AABox *b) {
  if (!a || !b) {
    return a;
  }

  Point amin = a->pmin, amax = a->pmax, bmin = b->pmin, bmax = b->pmax;

  // b is inside a
  if (vec3_le(amin, bmin) && vec3_le(bmax, amax)) {
    return a;
  }

  Point pmin = V(MIN(amin.x, bmin.x), MIN(amin.y, bmin.y), MIN(amin.z, bmin.z));
  Point pmax = V(MAX(amax.x, bmax.x), MAX(amax.y, bmax.y), MAX(amax.z, bmax.z));

  AABox *result = aabox_new(pmin, pmax, shpt_get(b->material));
  aabox_free((Surface *)a);

  return result;
}

void aabox_free(Surface *s) {
  AABox *self = (AABox *)s;
  Plane **sides = self->sides;

  for (uint i = 0; i < arr_len(sides); i++)
    plane_free((Surface *)sides[i]);

  arr_free(sides);

  if (self->material)
    shpt_free(self->material);

  free(self);
}

/**
 * @brief Widens the given AABox if it's too thin on any axis
 *
 * @param b AABox to widen
 * @return An AABox that's at least EPS wide in all it's axis
 */
AABox *aabox_widen_if_too_thin(AABox *b) {
  Vec3 sizes = aabox_sizes(b);

  // b is wide enough in all its axis
  if (sizes.x >= EPS && sizes.y >= EPS && sizes.z >= EPS)
    return b;

  Point pmin = b->pmin, pmax = b->pmax;

  if (sizes.x < EPS) {
    pmin.x = (pmin.x + pmax.x - EPS) / 2;
    pmax.x = (pmin.x + pmax.x + EPS) / 2;
  }

  if (sizes.y < EPS) {
    pmin.y = (pmin.y + pmax.y - EPS) / 2;
    pmax.y = (pmin.y + pmax.y + EPS) / 2;
  }

  if (sizes.z < EPS) {
    pmin.z = (pmin.z + pmax.z - EPS) / 2;
    pmax.z = (pmin.z + pmax.z + EPS) / 2;
  }

  // Create bigger AABox first to prevent material of freeing if it's the last reference
  AABox *bigger = aabox_new(pmin, pmax, b->material);
  aabox_free((Surface *)b);

  return bigger;
}

Triangle *triangle_new(Point p1, Point p2, Point p3, Material *material) {
  Triangle *t = malloc(sizeof(Triangle));
  Point pmin = V(MIN3(p1.x, p2.x, p3.x), MIN3(p1.y, p2.y, p3.y), MIN3(p1.z, p2.z, p3.z));
  Point pmax = V(MAX3(p1.x, p2.x, p3.x), MAX3(p1.y, p2.y, p3.y), MAX3(p1.z, p2.z, p3.z));
  AABox *b = aabox_widen_if_too_thin(aabox_new(pmin, pmax, NULL));

  *t = (Triangle){{.type = TRIANGLE, .bounding_box = b, .reference = b->base.reference},
                  p1,
                  p2,
                  p3,
                  shpt_get(material)};

  return t;
}

void triangle_free(Surface *s) {
  shpt_free(((Triangle *)s)->material);
  aabox_free((Surface *)s->bounding_box);
  free(s);
}

char axis = 'x';

int sort_objects_by_axis(const void *a, const void *b) {
  Point aref = ((Surface *)a)->reference;
  Point bref = ((Surface *)b)->reference;

  if (axis == 'x')
    return aref.x < bref.x ? -1 : 1;
  else if (axis == 'y')
    return aref.y < bref.y ? -1 : 1;
  else
    return aref.z < bref.z ? -1 : 1;
}

char largest_axis(const AABox *b) {
  Vec3 sizes = aabox_sizes(b);

  if (sizes.x > sizes.y) {
    return sizes.x > sizes.z ? 'x' : 'z';
  } else {
    return sizes.y > sizes.z ? 'y' : 'z';
  }
}

void list_shallow_free(List *l) {
  arr_free(l->surfaces);
  free(l);
}

KDTreeNode *kdtree_node_new(Surface **objects, uint n_leafs) {
  KDTreeNode *n = malloc(sizeof(KDTreeNode));
  List *object_list = list_new(objects);
  AABox *b = object_list->base.bounding_box;

  *n = (KDTreeNode){
    // even if it's not actually a KDTREE, it doesn't matter since we don't really use it
    {.type = KDTREE, .bounding_box = b, .reference = object_list->base.reference},
    NULL,
    NULL,
    object_list};

  // we need to separate the objects in left and right nodes
  if (arr_len(objects) > n_leafs) {
    // sort objects by its' positions in the largest axis
    axis = largest_axis(b);
    uint len = arr_len(objects);
    qsort(objects, len, sizeof(Surface *), sort_objects_by_axis);

    // separate in left and right objects
    uint middle = len / 2;
    Surface **left_objects = NULL, **right_objects = NULL;

    // grow arrays right away since we known the final size
    arr_grow(left_objects, middle);
    arr_grow(right_objects, len - middle);

    uint i = 0;
    for (; i < middle; i++)
      arr_push(left_objects, objects[i]);

    for (; i < len; i++)
      arr_push(right_objects, objects[i]);

    list_shallow_free(object_list);

    n->left = kdtree_node_new(left_objects, n_leafs);
    n->right = kdtree_node_new(right_objects, n_leafs);
    n->objects = NULL;
  }

  return n;
}

void kdtree_node_free(KDTreeNode *n) {
  // it's a leaf node, then we free its objects
  if (n->objects) {
    // we don't need to free n->bounding_box since it's = to n->objects->bounding_box
    list_free((Surface *)n->objects);
  } else { // it's a parent node, then we free its childs
    // if we're not freeing n->objects, then we need to free n->bounding_box
    aabox_free((Surface *)n->base.bounding_box);
    kdtree_node_free(n->left);
    kdtree_node_free(n->right);
  }

  free(n);
}

Surface **read_triangles(const char *path, Material *material) {
  Surface **triangles = NULL;
  char buf[MAX_BUF_SIZE];
  FILE *fp = fopen(path, "r");
  Vec3 *vertices = NULL;

  while (fgets(buf, MAX_BUF_SIZE, fp)) {
    char *c = buf;

    switch (c[0]) {
    case 'v':
      if (c[1] == ' ') {
        c += 2;
        arr_push(vertices, parse_vec3(c, NULL));
      }
      break;
    case 'f':
      if (c[1] == ' ') {
        c += 2; // skip to values
        uint i, j, k;

        i = strtold(c, &c);
        c = strfind(c, ' ') + 1; // skip vn index
        j = strtold(c, &c);
        c = strfind(c, ' ') + 1;
        k = strtold(c, NULL);

        arr_push(triangles, (Surface *)triangle_new(vertices[i - 1], vertices[j - 1],
                                                    vertices[k - 1], material));
      }
      break;
    }
  }

  arr_free(vertices);
  fclose(fp);

  return triangles;
}
KDTree *kdtree_new(Surface **objects, uint n_leafs) {
  KDTree *t = malloc(sizeof(KDTree));
  KDTreeNode *root = kdtree_node_new(objects, n_leafs);
  AABox *b = root->base.bounding_box;

  *t =
    (KDTree){{.type = KDTREE, .bounding_box = b, .reference = b->base.reference}, root};

  return t;
}

void kdtree_free(Surface *t) {
  kdtree_node_free(((KDTree *)t)->root);
  free(t);
}

KDTree *triangle_mesh_new_from_file(const char *path, uint n_leafs, Material *material) {
  return kdtree_new(read_triangles(path, material), n_leafs);
}

#ifndef SURFACES_H
#define SURFACES_H

#include "materials.h"
#include "util.h"

enum surface_type {
  PLANE = 0,
  SPHERE,
  LIST,
  AABOX,
  TRIANGLE,
  KDTREE,
  SURFACE_TYPE_SIZE
};

typedef struct _aabox AABox;

typedef struct {
  enum surface_type type;
  /** Axis-aligned box that encloses the surface. NULL if it can't be enclosed */
  AABox *bounding_box;
  /** Reference point for the surface. Should indicate its position roughly */
  Point reference;
} Surface;

typedef struct {
  Surface base;
  Point origin;
  Vec3 normal;
  Material *material;
} Plane;

typedef struct {
  Surface base;
  Point center;
  real radius;
  Material *material;
} Sphere;

typedef struct {
  Surface base;
  Surface **surfaces;
  uint count_by_type[SURFACE_TYPE_SIZE];
} List;

typedef struct {
  Surface base;
  Point p1, p2, p3;
  Material *material;
} Triangle;

enum plane { YZ = 0, ZX, XY };

struct _aabox {
  Surface base;
  /** Corner of the box with the lowest coordinates */
  Point pmin;
  /** Corner of the box opposite of pmin */
  Point pmax;
  Plane **sides;
  Material *material;
};

struct _node;

typedef struct _node {
  Surface base;
  struct _node *left, *right;
  List *objects;
} KDTreeNode;

typedef struct {
  Surface base;
  KDTreeNode *root;
} KDTree;

Plane *plane_new(Point, Vec3, Material *);
void plane_free(Surface *);

Sphere *sphere_new(Point, real, Material *);
void sphere_free(Surface *);

List *list_new(Surface **);
void list_free(Surface *);

AABox *aabox_new(Point, Point, Material *);
void aabox_free(Surface *);

Triangle *triangle_new(Point, Point, Point, Material *);
void triangle_free(Surface *);

KDTree *kdtree_new(Surface **, uint);
void kdtree_free(Surface *);

KDTree *triangle_mesh_new_from_file(const char *, uint, Material *);

#endif /* end of include guard: SURFACES_H */

#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "List.h"

typedef struct {
  Hittable _hittable;
  List *objects;
  shrmat sm;
} TriangleMesh;

Hittable *triangle_mesh_init(const char *path, shrmat sm);
void triangle_mesh_destroy(Hittable *);

#endif // TRIANGLEMESH_H

#include "TriangleMesh.h"

#include "../array.h"
#include "../util.h"
#include "Triangle.h"
#include <stdio.h>
#include <string.h>

bool triangle_mesh_hit(const Hittable *o, const ray *r, double t_min, double t_max, Record *hr);
void triangle_mesh_destroy(Hittable *self);
Box *triangle_mesh_bbox(const Hittable *h);

Hittable *triangle_mesh_init(const char *path, shrmat sm) {
  TriangleMesh *self = malloc(sizeof(TriangleMesh));

  self->_hittable.hit = triangle_mesh_hit;
  self->_hittable.destroy = triangle_mesh_destroy;
  self->_hittable.bbox = triangle_mesh_bbox;

  sm.refcount++;
  self->sm = sm;

  self->objects = (List *)list_init();

  char buf[MAX_BUF_SIZE];
  FILE *fp = fopen(path, "r");

  array_vec3 *vertex = array_vec3_init();

  while (fgets(buf, MAX_BUF_SIZE, fp)) {
    char *c = buf;

    switch (c[0]) {
    case 'v':
      if (c[1] == ' ') {
        c += 2;
        array_vec3_push(vertex, parse_vec3(c, NULL));
      }
      break;
    case 'f':
      if (c[1] == ' ') {
        c += 2; // skip to values
        size_t i, j, k;

        i = strtold(c, &c);
        c = strfind(c, ' ') + 1; // skip vn index
        j = strtold(c, &c);
        c = strfind(c, ' ') + 1;
        k = strtold(c, NULL);

        list_push(self->objects,
                  triangle_init(array_vec3_get(vertex, i - 1), array_vec3_get(vertex, j - 1),
                                array_vec3_get(vertex, k - 1), sm));
      }
      break;
    }
  }

  array_vec3_destroy(vertex);
  fclose(fp);

  return (Hittable *)self;
}

bool triangle_mesh_hit(const Hittable *o, const ray *r, double t_min, double t_max, Record *hr) {
  const Hittable *objects = (Hittable *)((TriangleMesh *)o)->objects;

  return objects->hit(objects, r, t_min, t_max, hr);
}

void triangle_mesh_destroy(Hittable *h) {
  TriangleMesh *self = (TriangleMesh *)h;

  DESTROY(self->objects);

  if (!--self->sm.refcount && self->sm.m)
    free(self->sm.m);

  free(self);
}

Box *triangle_mesh_bbox(const Hittable *h) { return ((TriangleMesh *)h)->objects->bbox; }

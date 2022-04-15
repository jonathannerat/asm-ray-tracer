#include <stdio.h>
#include <sys/types.h>

#include "Scene.h"
#include "hittable/Box.h"
#include "hittable/List.h"
#include "hittable/Plane.h"
#include "hittable/Sphere.h"
#include "hittable/Triangle.h"

int main(int argc, char **argv) {
  Scene *s;
  List *l = (List *)list_init();

  if (argc > 1)
    s = scene_init_file(argv[1]);
  else
    s = scene_init();

  // list_push(l, (Hittable *)triangle_init((point){0, 1, 0}, (point){1, 0, 0}, (point){0, 0, 1},
  //                                        (Material *)lambertian_init((color){1, 1, .2})));
  list_push(l, plane_init((point){0, -.2, 0}, (vec3){0, 1, 0}, lambertian_init((color){.3, .8, .1})));
  list_push(l, box_init((point){2, 1, 0}, (point){0, 0, 1}, lambertian_init((color){.1, .2, .6})));
  list_push(l, sphere_init((point){-2, 1, 0}, .9, dielectric_init((color){1, 1, 1}, .95)));
  list_push(l, sphere_init((point){-2, 1, 0}, -.85, dielectric_init((color){1, 1, 1}, .95)));

  s->world = (Hittable *)l;

  scene_render(s);

  DESTROY(l);
  free(s);

  return 0;
}

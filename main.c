#include <stdio.h>
#include <sys/types.h>

#include "Scene.h"
#include "hittable/Box.h"
#include "hittable/KDTree.h"
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

  list_push(
    l, kdtree_init((List *)list_parse_obj("models/cow.obj", lambertian_init((color){1, .2, .15})), 500));

  s->world = (Hittable *)l;

  scene_render(s);

  DESTROY(l);
  free(s);

  return 0;
}

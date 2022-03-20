#include <stdio.h>
#include <sys/types.h>

#include "Scene.h"
#include "hittable/List.h"
#include "hittable/Plane.h"
#include "util.h"

int main(int argc, char **argv) {
  Scene *s;
  List *l = list_init();

  if (argc > 1)
    s = scene_init_file(argv[1]);
  else
    s = scene_init();

  list_push(l, (Hittable *)plane_init((vec3){0, 0, 0}, (vec3){0, 1, 0}));

  s->world = (Hittable *)l;

  scene_render(s);

  list_destroy(l);
  scene_destroy(s);

  return 0;
}

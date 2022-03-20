#include <stdio.h>
#include <sys/types.h>

#include "Scene.h"
#include "hittable/Box.h"
#include "hittable/List.h"
#include "hittable/Plane.h"
#include "hittable/Sphere.h"

int main(int argc, char **argv) {
  Scene *s;
  List *l = list_init();

  if (argc > 1)
    s = scene_init_file(argv[1]);
  else
    s = scene_init();

  list_push(l, (Hittable *)sphere_init((point){0, 1, 0}, .9));

  s->world = (Hittable *)l;

  scene_render(s);

  list_destroy(l);
  scene_destroy(s);

  return 0;
}

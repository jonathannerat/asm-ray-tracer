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

  if (argc > 1)
    s = scene_init_file(argv[1]);
  else
    s = scene_init();

  scene_render(s);

  DESTROY(s->world);
  free(s);

  return 0;
}

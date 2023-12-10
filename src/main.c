#include "Scene.h"
#include "util.h"
#include <stdio.h>

int main(int argc, char **argv) {
  Scene *s;

  if (argc > 1)
    s = scene_new_from_file(argv[1]);
  else
    s = scene_new();

  scene_render(s);
  scene_dump(s, stdout);
  scene_free(s);

  return 0;
}

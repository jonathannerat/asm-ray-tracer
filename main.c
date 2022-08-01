#include "Scene.h"
#include "util.h"

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

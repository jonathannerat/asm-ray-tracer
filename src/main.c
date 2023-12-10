#include "Scene.h"
#include "util.h"
#include <stdio.h>

int main(int argc, char **argv) {
  Scene *s;

  if (argc > 1)
    s = scene_new_from_file(argv[1]);
  else
    s = scene_new();

  Color *image = malloc(sizeof(Color) * s->output.height * s->output.width);
  scene_render(s, image);
  dump_image(stdout, s, image);
  free(image);

  DESTROY(s->world);
  free(s);

  return 0;
}

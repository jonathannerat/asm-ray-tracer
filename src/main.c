#include "scene/Scene.h"
#include "structures/array.h"
#include "tracer.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

enum tracer_type { C, ASM };

typedef struct {
  char *scene_path;
  enum tracer_type tracer_type;
} Config;

Config parse_config(int argc, char **argv);

int main(int argc, char **argv) {
  Scene *s;
  Config c = parse_config(argc, argv);

  if (c.scene_path)
    s = scene_new_from_file(c.scene_path);
  else
    s = scene_new_from_stdin();

  if (c.tracer_type == C)
    tracer_c(s);
  else
    tracer_asm(s->camera, s->output, s->world, s->framebuffer);

  scene_dump(s, stdout);
  scene_free(s);

  return 0;
}

Config parse_config(int argc, char **argv) {
  Config config = {NULL, C};
  // mtsrand(0);

  for (int i = 1; i < argc;) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'c':
        config.tracer_type = C;
        i++;
        break;
      case 'a':
        config.tracer_type = ASM;
        i++;
        break;
      case 's':
        config.scene_path = argv[i + 1];
        i += 2;
        break;
      }
    } else {
      fprintf(stderr, "Invalid option: %s", argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  return config;
}

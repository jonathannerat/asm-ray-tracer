#include "scene/Scene.h"
#include "structures/array.h"
#include "tracer.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_SEED 0x20221218

enum tracer_type { C, ASM };

typedef struct {
  char *scene_path;
  enum tracer_type tracer_type;
  int seed;
} Config;

Config parse_config(int argc, char **argv);
void print_usage(char *);

int main(int argc, char **argv) {
  Scene *s;
  Config c = parse_config(argc, argv);

  if (c.scene_path)
    s = scene_new_from_file(c.scene_path);
  else
    s = scene_new_from_stdin();

  mtsrand(c.seed);

  if (c.tracer_type == C)
    tracer_c(s);
  else
    tracer_asm(s->camera, s->output, s->world, s->framebuffer);

  scene_dump(s, stdout);
  scene_free(s);

  return 0;
}

Config parse_config(int argc, char **argv) {
  Config config = {NULL, C, DEFAULT_SEED};

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
      case 'r':
        config.seed = atoi(argv[i+1]);
        i += 2;
        break;
      case 'h':
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
        break;
      default:
        fprintf(stderr, "Invalid option: %s", argv[i]);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
        break;
      }
    } else {
      fprintf(stderr, "Invalid option: %s", argv[i]);
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  return config;
}

void print_usage(char *binary) {
  fprintf(stderr, "Usage: %s [-c|-a] [-s SCENE] [-r SEED]\n", binary);
  fprintf(stderr, "-c        Run using C implementation (default)\n"
                  "-a        Run using ASM implementation\n"
                  "-s SCENE  Render the scene described in SCENE\n"
                  "-r SEED   Seed the RNG with SEED\n");
}

#ifndef SCENE_H
#define SCENE_H

#include <stdio.h>

#include "../surfaces.h"
#include "Camera.h"

typedef struct {
  uint width;
  uint height;
  uint samples_per_pixel;
  uint max_depth;
} Output;

typedef struct {
  Camera camera;
  Output output;
  List *world;
  Color *framebuffer;
} Scene;

Scene *scene_new_from_stdin();
Scene *scene_new_from_file(const char *path);
void scene_dump(Scene *s, FILE *f);
void scene_free(Scene *s);

#endif /* end of include guard: SCENE_H */

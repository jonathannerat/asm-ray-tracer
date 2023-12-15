#ifndef SCENE_H
#define SCENE_H

#include <stdio.h>

#include "Camera.h"
#include "../surfaces.h"

typedef struct {
  int width;
  int height;
  int samples_per_pixel;
  int max_depth;
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

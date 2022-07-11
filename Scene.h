#include "core.h"

typedef struct {
  int width;
  int height;
  int samples_per_pixel;
  int max_depth;
} output;

typedef struct {
  Camera camera;
  output output;
  Hittable *world;
} Scene;

Scene *scene_init();
Scene *scene_init_file(const char* path);
void scene_render(const Scene *s);

#include <sys/types.h>

#include "camera.h"
#include "Hittable.h"

typedef struct {
  u_int16_t width;
  u_int16_t height;
  u_int16_t samples_per_pixel;
  u_int16_t max_depth;
} output;

typedef struct {
  camera camera;
  output output;
  Hittable *world;
} Scene;

Scene *scene_init();
Scene *scene_init_file(const char* path);
void scene_render(const Scene *s);
inline void scene_destroy(Scene *s) { free(s); }

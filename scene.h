#include <sys/types.h>

#include "camera.h"

typedef struct {
  u_int16_t width;
  u_int16_t height;
  u_int16_t samples_per_pixel;
  u_int16_t max_depth;
} output;

typedef struct {
  camera camera;
  output output;
} scene;

scene scene_init();

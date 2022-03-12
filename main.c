#include <stdio.h>
#include <sys/types.h>

#include "util.h"

typedef struct {
  u_int16_t width;
  u_int16_t height;
  u_int16_t samples_per_pixel;
  u_int16_t max_depth;
} output;


int main(int argc, char **argv) {
  ray r = {
    .origin = {1,0,0},
    .direction = {.5,1,0},
  };

  vec3_debug(ray_at(&r, 2));
  return 0;
}

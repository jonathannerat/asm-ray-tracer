#include <stdio.h>
#include <sys/types.h>

#include "ray.h"

typedef struct {
  u_int16_t width;
  u_int16_t height;
  u_int16_t samples_per_pixel;
  u_int16_t max_depth;
} output;

void vec3_debug(const vec3);
void ray_debug(const ray*);

int main(int argc, char **argv) {
  ray r = {
      .origin = {0, 0, 0},
      .direction = {0, 1, 0},
  };

  ray_debug(&r);
  return 0;
}

void vec3_debug(const vec3 v) {
  fprintf(stderr, "(%f, %f, %f)", v.x, v.y, v.z);
}

void ray_debug(const ray *r) {
  fprintf(stderr, "ray { ");
  vec3_debug(r->origin);
  fprintf(stderr, " -> ");
  vec3_debug(r->direction);
  fprintf(stderr, " }");
}


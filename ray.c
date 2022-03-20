#include <string.h>

#include "ray.h"

char ray_buf[RAY_BUF_SIZE];

const char* ray_tostr(const ray *r) {
  u_int32_t size;
  snprintf(ray_buf, RAY_BUF_SIZE, "ray {%s", vec3_tostr(r->origin));
  size = strlen(ray_buf);
  snprintf(ray_buf + size, RAY_BUF_SIZE - size, " -> %s}", vec3_tostr(r->direction));
  return ray_buf;
}

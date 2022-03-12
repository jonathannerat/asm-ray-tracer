#include "vec3.h"

#include <stdarg.h>

vec3 vec3_sum(const u_int32_t n, ...) {
  vec3 r = {.x = 0, .y = 0, .z = 0};
  u_int32_t i = 0;
  va_list args;

  va_start(args, n);

  for (; i < n; i++) {
    vec3 v = va_arg(args, vec3);
    r.x += v.x;
    r.y += v.y;
    r.z += v.z;
  }

  va_end(args);

  return r;
}

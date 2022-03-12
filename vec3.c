#include <stdarg.h>

#include "vec3.h"
#include "util.h"

vec3 vec3_normalized(const vec3 v) {
  return vec3_scale(1 / sqrt(vec3_norm2(v)), v);
}

vec3 vec3_random_in_unit_sphere() {
  while (1) {
    vec3 p = vec3_random_between(-1, 1);

    if (vec3_norm2(p) >= 1) 
      continue;

    return p;
  }
}

vec3 vec3_random() {
  return (vec3){random_double(), random_double(), random_double()};
}

vec3 vec3_random_between(double min, double max) {
  return (vec3){random_double_between(min, max),
                random_double_between(min, max),
                random_double_between(min, max)};
}

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

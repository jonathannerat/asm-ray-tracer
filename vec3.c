#include <stdarg.h>
#include <stdio.h>

#include "util.h"
#include "vec3.h"

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

vec3 parse_vec3(char *c, char **t) {
  vec3 v;

  v.x = strtod(c, &c);
  c++;

  v.y = strtod(c, &c);
  c++;

  v.z = strtod(c, &c);

  if (t)
    *t = c;

  return v;
}

char vec3_buf[VEC3_BUF_SIZE];

const char *vec3_tostr(const vec3 v) {
  snprintf(vec3_buf, VEC3_BUF_SIZE, "(%.3f, %.3f, %.3f)", v.x, v.y, v.z);
  return vec3_buf;
}

void write_color(color pixel, u_int16_t spp) {
  double r = pixel.x;
  double g = pixel.y;
  double b = pixel.z;
  double scale = 1.0 / spp;

  r = sqrt(r * scale);
  g = sqrt(g * scale);
  b = sqrt(b * scale);

  printf("%d %d %d\n", (int)(256 * clamp(r, 0, 1)), (int)(256 * clamp(g, 0, 1)),
         (int)(256 * clamp(b, 0, 1)));
}

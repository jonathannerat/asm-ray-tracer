#include "util.h"
#include <stdio.h>

#ifdef DEBUG
#include <stdarg.h>
void d(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}
#else
void d(const char *fmt, ...) {}
#endif

char* strfind(char *c, char f) {
  while(*c && *c != f) c++;

  return c;
}

Vec3 parse_vec3(char *c, char **t) {
  Vec3 v;

  v.x = strtod(c, &c);
  c++;

  v.y = strtod(c, &c);
  c++;

  v.z = strtod(c, &c);

  if (t)
    *t = c;

  return v;
}

void write_color(Color pixel, uint spp) {
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

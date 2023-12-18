#include "util.h"
#include "vec3.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdarg.h>
void d(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}
#else
void d(const char *fmt, ...) { (void)fmt; }
#endif

real frand() { return rand() / (RAND_MAX + 1.0); }

char *strfind(char *c, char f) {
  while (*c && *c != f)
    c++;

  return c;
}

static real clamp(real x, real min, real max) {
  return x < min ? min : (x > max ? max : x);
}

void write_color(Color pixel, uint spp) {
  real r = pixel.x;
  real g = pixel.y;
  real b = pixel.z;
  real scale = 1.0 / spp;

  r = sqrt(r * scale);
  g = sqrt(g * scale);
  b = sqrt(b * scale);

  printf("%d %d %d\n", (int)(256 * clamp(r, 0, 1)), (int)(256 * clamp(g, 0, 1)),
         (int)(256 * clamp(b, 0, 1)));
}

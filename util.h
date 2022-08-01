#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdlib.h>

#include "core.h"

void d(const char *fmt, ...);

#ifdef DEBUG
#define D(...) d("[debug] " __VA_ARGS__)
#else
#define D(...) d(NULL)
#endif

#define EPS 1e-4
#define M_PI 3.14159265358979323846f
#define MAX_BUF_SIZE 256

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define DESTROY(e) ((Hittable *)(e))->destroy((Hittable *)e)
#define HIT(e, ...) ((Hittable *)(e))->hit((Hittable *)(e), __VA_ARGS__)
#define V(vx, vy, vz) ((Vec3) {.x = (vx), .y = (vy), .z = (vz), ._ = 0})

inline real degrees_to_radians(real degrees) { return degrees * M_PI / 180.0; }

real rnd();

inline real rnd_between(real min, real max) {
  return min + rnd() * (max - min);
}

char* strfind(char *c, char f);
Vec3 parse_vec3(char *c, char **t);
void write_color(Color pixel, uint spp);

#endif // UTIL_H

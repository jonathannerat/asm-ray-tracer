#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void d(const char *fmt, ...);

#ifdef DEBUG
#define D(...) d("[debug] " __VA_ARGS__)
#else
#define D(...) d(NULL)
#endif

typedef unsigned int uint;
typedef float real;

typedef struct {
  real x;
  real y;
  real z;
  real _; // padding
} Vec3;

typedef Vec3 Point;
typedef Vec3 Color;

typedef struct {
  Point origin;
  Vec3 direction;
} Ray;

#define EPS 1e-4
#define MAX_BUF_SIZE 256

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MIN3(a, b, c) (MIN(MIN((a), (b)), (c)))
#define MAX3(a, b, c) (MAX(MAX((a), (b)), (c)))
#define DESTROY(e) ((Hittable *)(e))->destroy((Hittable *)e)
#define HIT(e, ...) ((Hittable *)(e))->hit((Hittable *)(e), __VA_ARGS__)
#define SCATTER(m, ...) ((Material *)(m))->scatter((Material *)(m), __VA_ARGS__)
#define V(vx, vy, vz) ((Vec3){.x = (vx), .y = (vy), .z = (vz), ._ = 0})
#define V3(v) (V(v, v, v))
#define VEC_AT(v, p) (((float *)(&v))[p])

real rnd();
char *strfind(char *c, char f);
Vec3 parse_vec3(char *c, char **t);
void write_color(Color pixel, uint spp);

Point ray_at(const Ray *r, real t);
#endif // UTIL_H

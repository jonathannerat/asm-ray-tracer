#ifndef VEC3_H
#define VEC3_H

#include <stdbool.h>

#define V(vx, vy, vz) ((Vec3){.x = (vx), .y = (vy), .z = (vz), ._ = 0})
#define V3(v) (V(v, v, v))
#define VEC_AT(v, p) (((real *)(&v))[p])

typedef float real;
typedef unsigned int uint;

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

Vec3 vec3_add(const Vec3 a, const Vec3 b);
Vec3 vec3_sub(const Vec3 a, const Vec3 b);
Vec3 vec3_prod(const Vec3 a, const Vec3 b);
Vec3 vec3_scale(const Vec3 a, real);
Vec3 vec3_unscale(const Vec3 a, real);
Vec3 vec3_inv(const Vec3 v);
real vec3_norm2(const Vec3 v);
real vec3_dot(const Vec3 a, const Vec3 b);
Vec3 vec3_to_normalized(const Vec3 v);
Vec3 vec3_cross(const Vec3 a, const Vec3 b);
Vec3 vec3_rand_unit_sphere();
bool vec3_le(const Vec3, const Vec3);

Point ray_at(const Ray *r, real t);
Vec3 parse_vec3(char *c, char **t);
void write_color(Color pixel, uint spp);

#endif /* end of include guard: VEC3_H */

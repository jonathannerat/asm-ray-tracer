#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <sys/types.h>

#include "util.h"

#define VEC3_BUF_SIZE 100

typedef struct {
  double x;
  double y;
  double z;
} vec3;

typedef vec3 point;
typedef vec3 color;

vec3 vec3_random_in_unit_sphere();
vec3 vec3_random();
vec3 vec3_random_between(double min, double max);
vec3 vec3_sum(const u_int32_t n, ...);
vec3 parse_vec3(char *c, char **t);

inline vec3 vec3_add(const vec3 a, const vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

inline vec3 vec3_inv(const vec3 v) { return (vec3){-v.x, -v.y, -v.z}; }

inline vec3 vec3_prod(const vec3 a, const vec3 b) {
  return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

inline vec3 vec3_scale(double s, const vec3 v) {
  return (vec3){v.x * s, v.y * s, v.z * s};
}

inline vec3 vec3_sub(const vec3 a, const vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

inline double vec3_norm2(const vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline vec3 cross(const vec3 a, const vec3 b) {
  return (vec3){
      a.y * b.z - b.y * a.z,
      b.x * a.z - a.x * b.z,
      a.x * b.y - b.x * a.y,
  };
}

inline double dot(const vec3 a, const vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline u_int8_t perpendicular(const vec3 a, const vec3 b) {
  return fabs(dot(a, b)) < EPS;
}

inline vec3 normalized(const vec3 v) {
  return vec3_scale(1.0 / sqrt(vec3_norm2(v)), v);
}

inline bool_ vec3_near_zero(const vec3 v) {
  return fabs(v.x) < EPS && fabs(v.y) < EPS && fabs(v.z) < EPS ;
}

const char* vec3_tostr(const vec3 v);

void write_color(color pixel, u_int16_t spp);

#endif // VEC3_H

#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <sys/types.h>

typedef struct {
  double x;
  double y;
  double z;
} vec3;

typedef vec3 point;

vec3 vec3_sum(const u_int32_t n, ...);
vec3 vec3_random_in_unit_sphere();
vec3 vec3_random();
vec3 vec3_random_between(double min, double max);

inline vec3 vec3_add(const vec3 a, const vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

inline vec3 vec3_cross(const vec3 a, const vec3 b) {
  return (vec3){
      a.y * b.z - b.y * a.z,
      b.x * a.z - a.x * b.z,
      a.x * b.y - b.x * a.y,
  };
}

inline double vec3_dot(const vec3 a, const vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 vec3_inv(const vec3 v) { return (vec3){-v.x, -v.y, -v.z}; }

inline vec3 vec3_scale(double s, const vec3 v) {
  return (vec3){v.x * s, v.y * s, v.z * s};
}

inline vec3 vec3_sub(const vec3 a, const vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

inline double vec3_norm2(const vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline vec3 vec3_normalized(const vec3 v) {
  return vec3_scale(1 / sqrt(vec3_norm2(v)), v);
}

#endif // VEC3_H

#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <stdlib.h>

typedef struct {
  float x;
  float y;
  float z;
} vec3;

typedef vec3 point;

vec3 vec3_sum(const u_int32_t n, ...);

inline vec3 vec3_add(const vec3 a, const vec3 b) {
  return (vec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

inline vec3 vec3_cross(const vec3 a, const vec3 b) {
  return (vec3){
      .x = a.y * b.z - b.y * a.z,
      .y = b.x * a.z - a.x * b.z,
      .z = a.x * b.y - b.x * a.y,
  };
}

inline float vec3_dot(const vec3 a, const vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 vec3_inv(const vec3 v) {
  return (vec3){.x = -v.x, .y = -v.y, .z = -v.z};
}

inline vec3 vec3_scale(float s, const vec3 v) {
  return (vec3){.x = v.x * s, .y = v.y * s, .z = v.z * s};
}

inline vec3 vec3_sub(const vec3 a, const vec3 b) {
  return (vec3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

inline float vec3_norm2(const vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline vec3 vec3_normalized(const vec3 v) {
  return vec3_scale(1 / sqrt(vec3_norm2(v)), v);
}

#endif // VEC3_H

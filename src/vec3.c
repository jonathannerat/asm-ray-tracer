#include "vec3.h"
#include "util.h"
#include <math.h>
#include <stdlib.h>

Vec3 vec3_add(const Vec3 a, const Vec3 b) { return V(a.x + b.x, a.y + b.y, a.z + b.z); }

Vec3 vec3_sub(const Vec3 a, const Vec3 b) { return V(a.x - b.x, a.y - b.y, a.z - b.z); }

Vec3 vec3_prod(const Vec3 a, const Vec3 b) { return V(a.x * b.x, a.y * b.y, a.z * b.z); }

Vec3 vec3_inv(const Vec3 v) { return V(-v.x, -v.y, -v.z); }

Vec3 vec3_scale(const Vec3 v, real s) { return V(v.x * s, v.y * s, v.z * s); }

Vec3 vec3_unscale(const Vec3 v, real s) { return V(v.x / s, v.y / s, v.z / s); }

real vec3_norm2(const Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

real vec3_dot(const Vec3 a, const Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 vec3_to_normalized(const Vec3 v) { return vec3_unscale(v, sqrt(vec3_norm2(v))); }

Vec3 vec3_cross(const Vec3 a, const Vec3 b) {
  return V(a.y * b.z - b.y * a.z, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y);
}

Vec3 vec3_rand_unit_sphere() {
  while (1) {
    Vec3 p = V(frand() * 2 - 1, frand() * 2 - 1, frand() * 2 - 1);

    if (vec3_norm2(p) >= 1)
      continue;

    return p;
  }
}

bool vec3_le(const Vec3 a, const Vec3 b) {
  return a.x <= b.x && a.y <= b.y && a.z <= b.z;
}

Point ray_at(const Ray *r, real t) {
  return vec3_add(r->origin, vec3_scale(r->direction, t));
}

Vec3 parse_vec3(char *c, char **t) {
  Vec3 v;

  v._ = 0;

  v.x = strtod(c, &c);
  c++;

  v.y = strtod(c, &c);
  c++;

  v.z = strtod(c, &c);

  if (t)
    *t = c;

  return v;
}

#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdlib.h>

#include "vec3.h"
#include "ray.h"

#define PI 3.1415926535897932385

inline double degrees_to_radians(double degrees) { return degrees * PI / 180.0; }

inline double random_double() { return rand() / (RAND_MAX + 1.0); }

inline double random_double_between(double min, double max) {
  return min + random_double() * (max - min);
}

void vec3_debug(const vec3 v);
void ray_debug(const ray *r);
char* strfind(char *c, char f);

#endif // UTIL_H

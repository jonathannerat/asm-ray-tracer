#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdlib.h>

#define PI 3.1415926535897932385

inline double degrees_to_radians(double degrees) { return degrees * PI / 180.0; }

inline double random_double() { return rand() / (RAND_MAX + 1.0); }

inline double random_double_between(double min, double max) {
  return min + random_double() * (max - min);
}

#endif // UTIL_H

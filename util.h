#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdlib.h>
#include <sys/types.h>

#define EPS 1e-7
#define M_PI 3.14159265358979323846f

#define false 0
#define true 1

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

typedef u_int8_t bool_;


inline double degrees_to_radians(double degrees) { return degrees * M_PI / 180.0; }

inline double random_double() { return rand() / (RAND_MAX + 1.0); }

inline double random_double_between(double min, double max) {
  return min + random_double() * (max - min);
}

inline double clamp(double x, double min, double max) {
  return x < min ? min : (x > max ? max : x);
}

char* strfind(char *c, char f);

#endif // UTIL_H

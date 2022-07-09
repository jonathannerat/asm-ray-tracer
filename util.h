#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

void d(const char *fmt, ...);

#ifdef DEBUG
#define D(...) d("[debug] " __VA_ARGS__)
#else
#define D(...) d(NULL)
#endif

#define EPS 1e-8
#define M_PI 3.14159265358979323846f

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define MAX_BUF_SIZE 256

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

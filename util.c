#include <stdio.h>

#include "util.h"

void vec3_debug(const vec3 v) {
  fprintf(stderr, "(%.2f, %.2f, %.2f)", v.x, v.y, v.z);
}

void ray_debug(const ray *r) {
  fprintf(stderr, "ray { ");
  vec3_debug(r->origin);
  fprintf(stderr, " -> ");
  vec3_debug(r->direction);
  fprintf(stderr, " }");
}

char* strfind(char *c, char f) {
  while(*c && *c != f) c++;

  return c;
}

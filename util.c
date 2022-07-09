#include "util.h"

#ifdef DEBUG
#include <stdio.h>
#include <stdarg.h>
void d(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}
#else
void d(const char *fmt, ...) {}
#endif

char* strfind(char *c, char f) {
  while(*c && *c != f) c++;

  return c;
}

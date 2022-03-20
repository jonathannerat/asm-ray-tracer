#include <stdio.h>

char* strfind(char *c, char f) {
  while(*c && *c != f) c++;

  return c;
}

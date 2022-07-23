#ifndef DEBUG_ASM
#include "Scene.h"
#endif

#include <math.h>
#include "array.h"
#include "hittable/Box.h"
#include "util.h"

int main(int argc, char **argv) {
  Ray r = {
    .origin = V(3, 1, 2.5),
    .direction = V(1, 0, 0),
  };
  Record rec;
  Hittable *p = box_init(V(2, 0, 2), V(4, 2, 3), NULL);

  bool hit = box_hit(p, &r, EPS, INFINITY, &rec);

  return hit;
}

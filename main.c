#ifndef DEBUG_ASM
#include "Scene.h"
#endif

#include <math.h>
#include "array.h"
#include "hittable/Triangle.h"
#include "util.h"

int main(int argc, char **argv) {
  // testing this scenario: https://www.geogebra.org/3d/uemetzau
  Ray r = {
    .origin = V(1.51,.54,0),
    .direction = V(-1.51,-.54,1),
  };
  Record rec;
  Hittable *p = triangle_init(V(-3,0,0), V(0,-2,0), V(0,0,2), NULL);
  bool hit = HIT(p, &r, EPS, INFINITY, &rec);

  return hit;
}

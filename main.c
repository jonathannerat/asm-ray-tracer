#ifndef DEBUG_ASM
#include "Scene.h"
#endif

#include <math.h>
#include "array.h"
#include "hittable/Sphere.h"
#include "util.h"

int main(int argc, char **argv) {
  // testing this scenario: https://www.geogebra.org/3d/sjetb69v
  Ray r = {
    .origin = V(-3.19,-1.78,.58),
    .direction = V(3.19,1.78,.42),
  };
  Record rec;
  Hittable *p = sphere_init(V(0,0,0), 1.5, NULL);
  bool hit = HIT(p, &r, EPS, INFINITY, &rec);

  return hit;
}

#ifndef MATERIAL_H
#define MATERIAL_H

#include "structures/shpt.h"
#include "vec3.h"

enum material_type { LAMBERTIAN, METAL, DIELECTRIC, LIGHT, MATERIAL_TYPE_SIZE };

typedef struct {
  Color color;
  real alpha;
  enum material_type type;
} Material;

#define material_new(TYPE, c, a)                                                         \
  ({                                                                                     \
    Material *m = shpt_new(Material);                                                    \
    *m = (Material){.type = TYPE, .color = c, .alpha = a};                               \
    m;                                                                                   \
  })
#define lambertian_new(c) material_new(LAMBERTIAN, c, 0)
#define metal_new(c, f) material_new(METAL, c, f)
#define dielectric_new(c, ir) material_new(DIELECTRIC, c, ir)
#define light_new(c) material_new(LIGHT, c, 0)

#endif // MATERIAL_H

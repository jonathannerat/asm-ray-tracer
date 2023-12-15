#ifndef SHADING_H
#define SHADING_H

#include "../materials.h"
#include "intersection.h"

Color material_emitted(Material *);

bool material_scatter(const Material *, const Ray *, const HitRecord *, Color *, Ray *);

#endif /* end of include guard: SHADING_H */

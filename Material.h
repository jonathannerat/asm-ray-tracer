#ifndef MATERIAL_H
#define MATERIAL_H

#include "core.h"

spmat *lambertian_init(Color albedo);
spmat *metal_init(Color albedo, double fuzz);
spmat *dielectric_init(Color albedo, double ir);
spmat *diffuse_light_init(Color albedo);

#endif // MATERIAL_H

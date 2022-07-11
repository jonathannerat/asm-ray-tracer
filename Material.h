#ifndef MATERIAL_H
#define MATERIAL_H

#include "core.h"

spmat *lambertian_init(Color albedo);
spmat *metal_init(Color albedo, real fuzz);
spmat *dielectric_init(Color albedo, real ir);
spmat *diffuse_light_init(Color albedo);

#endif // MATERIAL_H

#ifndef MATERIAL_H
#define MATERIAL_H

#include "core.h"

/// LAMBERTIAN

typedef struct {
  Material _material;
  color albedo;
} Lambertian;

spmat *lambertian_init(color albedo);

/// METAL

typedef struct {
  Material _material;
  color albedo;
  double fuzz;
} Metal;

spmat *metal_init(color albedo, double fuzz);

/// DIELECTRIC

typedef struct {
  Material _material;
  color albedo;
  double ir;
} Dielectric;

spmat *dielectric_init(color albedo, double ir);

typedef struct {
  Material _material;
  color albedo;
} DiffuseLight;

spmat *diffuse_light_init(color albedo);

#endif // MATERIAL_H

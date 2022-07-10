#ifndef MATERIAL_H
#define MATERIAL_H

#include "core.h"

/// LAMBERTIAN

typedef struct {
  Material _material;
  Color albedo;
} Lambertian;

spmat *lambertian_init(Color albedo);

/// METAL

typedef struct {
  Material _material;
  Color albedo;
  double fuzz;
} Metal;

spmat *metal_init(Color albedo, double fuzz);

/// DIELECTRIC

typedef struct {
  Material _material;
  Color albedo;
  double ir;
} Dielectric;

spmat *dielectric_init(Color albedo, double ir);

typedef struct {
  Material _material;
  Color albedo;
} DiffuseLight;

spmat *diffuse_light_init(Color albedo);

#endif // MATERIAL_H

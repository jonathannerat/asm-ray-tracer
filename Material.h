#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable/Record.h"

typedef struct _material Material;

typedef struct _spmat spmat;

struct _spmat {
  Material *m;
  size_t c;
};

struct _material {
  bool (*scatter)(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                  ray *scattered);
};

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

#endif // MATERIAL_H

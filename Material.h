#ifndef MATERIAL_H
#define MATERIAL_H

#include "ray.h"

typedef struct record Record;
typedef struct material Material;

typedef struct {
  Material *m;
  size_t refcount;
} shrmat;

typedef bool (*scatter_method)(const Material *m, const ray *r_in, const Record *hr, color *attenuation, ray *scattered);

struct material{
  scatter_method scatter;
};



/// LAMBERTIAN

typedef struct {
  Material _material;
  color albedo;
} Lambertian;

Material *lambertian_init(color albedo);



/// METAL

typedef struct {
  Material _material;
  color albedo;
  double fuzz;
} Metal;

Material *metal_init(color albedo, double fuzz);



/// DIELECTRIC

typedef struct {
  Material _material;
  color albedo;
  double ir;
} Dielectric;

Material *dielectric_init(color albedo, double ir);

#endif // MATERIAL_H

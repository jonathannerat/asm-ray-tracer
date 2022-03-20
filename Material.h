#ifndef MATERIAL_H
#define MATERIAL_H

#include "ray.h"

typedef struct record Record;
typedef struct material Material;

typedef bool_ (*scatter_method)(const Material *m, const ray *r_in, const Record *hr, color *attenuation, ray *scattered);

struct material{
  scatter_method scatter;
};



/// LAMBERTIAN

typedef struct {
  Material _material;
  color albedo;
} Lambertian;

Lambertian *lambertian_init(color albedo);
void lambertian_destroy(Lambertian *self);



/// METAL

typedef struct {
  Material _material;
  color albedo;
  double fuzz;
} Metal;

Metal *metal_init(color albedo, double fuzz);
void metal_destroy(Metal *self);

#endif // MATERIAL_H

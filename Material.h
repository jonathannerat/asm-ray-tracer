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

#endif // MATERIAL_H

#include "hittable/Record.h"

bool_ lambertian_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                         ray *scattered);

Lambertian *lambertian_init(color albedo) {
  Lambertian *m = malloc(sizeof(Lambertian));

  m->_material.scatter = lambertian_scatter;
  m->albedo = albedo;

  return m;
}

bool_ lambertian_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                         ray *scattered) {
  Lambertian *self = (Lambertian *)m;
  vec3 scatter_direction = vec3_add(hr->normal, vec3_random_in_unit_sphere());

  if (vec3_near_zero(scatter_direction))
    scatter_direction = hr->normal;

  *scattered = (ray){hr->p, scatter_direction};
  *attenuation = self->albedo;
  return true;
}

void lambertian_destroy(Lambertian *self) { free(self); }

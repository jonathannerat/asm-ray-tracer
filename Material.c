#include "Material.h"

double reflectance(double cosine, double ref_idx);

/// LAMBERTIAN

bool lambertian_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                        ray *scattered);

spmat *lambertian_init(color albedo) {
  Lambertian *mat = malloc(sizeof(Lambertian));
  spmat *sm = malloc(sizeof(spmat));

  mat->_material.scatter = lambertian_scatter;
  mat->albedo = albedo;

  sm->m = (Material *)mat;
  sm->c = 0;

  return sm;
}

bool lambertian_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                        ray *scattered) {
  Lambertian *self = (Lambertian *)m;
  vec3 scatter_direction = vec3_add(hr->normal, vec3_random_in_unit_sphere());

  if (vec3_near_zero(scatter_direction))
    scatter_direction = hr->normal;

  *scattered = (ray){hr->p, scatter_direction};
  *attenuation = self->albedo;
  return true;
}

/// METAL

bool metal_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                   ray *scattered);

spmat *metal_init(color albedo, double fuzz) {
  Metal *mat = malloc(sizeof(Metal));
  spmat *sm = malloc(sizeof(spmat));

  mat->_material.scatter = metal_scatter;
  mat->albedo = albedo;
  mat->fuzz = fuzz;

  sm->m = (Material *)mat;
  sm->c = 0;

  return sm;
}

bool metal_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                   ray *scattered) {
  Metal *self = (Metal *)m;
  vec3 reflected = reflect(normalized(r_in->direction), hr->normal);

  *scattered = (ray){
    hr->p,
    vec3_add(reflected, vec3_scale(self->fuzz, vec3_random_in_unit_sphere())),
  };
  *attenuation = self->albedo;

  return dot(reflected, hr->normal) > 0;
}

/// DIELECTRIC

bool dielectric_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                        ray *scattered);

spmat *dielectric_init(color albedo, double ir) {
  Dielectric *mat = malloc(sizeof(Dielectric));
  spmat *sm = malloc(sizeof(spmat));

  mat->_material.scatter = dielectric_scatter;
  mat->albedo = albedo;
  mat->ir = ir;

  sm->m = (Material *)mat;
  sm->c = 0;

  return sm;
}

bool dielectric_scatter(const Material *m, const ray *r_in, const Record *hr, color *attenuation,
                        ray *scattered) {
  Dielectric *self = (Dielectric *)m;
  double ref_ratio = hr->front_face ? 1 / self->ir : self->ir;
  vec3 unit_dir = normalized(r_in->direction);
  double cos_theta = fmin(dot(vec3_inv(unit_dir), hr->normal), 1);
  double sin_theta = sqrt(1 - cos_theta * cos_theta);
  bool cannot_refract = ref_ratio * sin_theta > 1;
  vec3 direction;

  if (cannot_refract || reflectance(cos_theta, ref_ratio) > random_double())
    direction = reflect(unit_dir, hr->normal);
  else
    direction = refract(unit_dir, hr->normal, ref_ratio);

  *attenuation = self->albedo;
  *scattered = (ray){hr->p, direction};

  return true;
}

double reflectance(double cosine, double ref_idx) {
  // Use Schlick's approximation for reflectance.
  double r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cosine), 5);
}

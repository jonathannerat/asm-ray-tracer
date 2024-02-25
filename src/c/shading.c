#include "shading.h"
#include "../vec3.h"
#include "../tracer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

bool lambertian_scatter(const Material *, const Ray *, const HitRecord *, Color *, Ray *);
bool metal_scatter(const Material *, const Ray *, const HitRecord *, Color *, Ray *);
bool dielectric_scatter(const Material *, const Ray *, const HitRecord *, Color *, Ray *);
bool vec3_near_zero(const Vec3 v);
Vec3 reflect(const Vec3 v, const Vec3 n);
Vec3 refract(const Vec3 uv, const Vec3 n, real etai_over_etat);
real reflectance(real cosine, real ref_idx);

Color material_emitted(Material *self) {
  if (self->type != LIGHT) {
    return V3(0);
  }

  return self->color;
}

bool material_scatter(const Material *self, const Ray *r, const HitRecord *hit,
                      Color *attenuation, Ray *scattered) {
  switch (self->type) {
  case LAMBERTIAN:
    return lambertian_scatter(self, r, hit, attenuation, scattered);
  case METAL:
    return metal_scatter(self, r, hit, attenuation, scattered);
  case DIELECTRIC:
    return dielectric_scatter(self, r, hit, attenuation, scattered);
  case LIGHT:
    return false;
  default:
    perror("Invalid material type");
    exit(EXIT_FAILURE);
  }
}

bool lambertian_scatter(const Material *self, const Ray *r, const HitRecord *hit,
                        Color *attenuation, Ray *scattered) {
  (void)r;
  Vec3 scatter_direction = vec3_add(hit->normal, vec3_rand_unit_sphere());

  if (vec3_near_zero(scatter_direction))
    scatter_direction = hit->normal;

  *attenuation = self->color;
  *scattered = (Ray){hit->p, scatter_direction};

  return true;
}

bool metal_scatter(const Material *self, const Ray *r, const HitRecord *hit,
                   Color *attenuation, Ray *scattered) {
  Vec3 reflected = reflect(vec3_to_normalized(r->direction), hit->normal);
  Vec3 scatter_dir = vec3_add(reflected, vec3_scale(vec3_rand_unit_sphere(), self->alpha));

  *attenuation = self->color;
  *scattered = (Ray){
    hit->p,
    scatter_dir,
  };

  return vec3_dot(reflected, hit->normal) > 0;
}

bool dielectric_scatter(const Material *self, const Ray *r, const HitRecord *hit,
                        Color *attenuation, Ray *scattered) {
  real ref_ratio = hit->front_face ? 1 / self->alpha : self->alpha;
  Vec3 unit_dir = vec3_to_normalized(r->direction);
  real cos_theta = fmin(vec3_dot(vec3_inv(unit_dir), hit->normal), 1);
  real sin_theta = sqrt(1 - cos_theta * cos_theta);
  bool cannot_refract = ref_ratio * sin_theta > 1;
  Vec3 direction;

  if (cannot_refract || reflectance(cos_theta, ref_ratio) > mtfrand())
    direction = reflect(unit_dir, hit->normal);
  else
    direction = refract(unit_dir, hit->normal, ref_ratio);

  *attenuation = self->color;
  *scattered = (Ray){hit->p, direction};

  return true;
}

bool vec3_near_zero(const Vec3 v) {
  return fabs(v.x) <= EPS && fabs(v.y) <= EPS && fabs(v.z) <= EPS;
}

Vec3 reflect(const Vec3 v, const Vec3 n) {
  return vec3_sub(v, vec3_scale(n, 2 * vec3_dot(v, n)));
}

Vec3 refract(const Vec3 uv, const Vec3 n, real etai_over_etat) {
  real cos_theta = fmin(vec3_dot(vec3_inv(uv), n), 1.0);
  Vec3 r_out_perp = vec3_scale(vec3_add(uv, vec3_scale(n, cos_theta)), etai_over_etat);
  Vec3 r_out_par = vec3_scale(n, -sqrt(fabs(1.0 - vec3_norm2(r_out_perp))));
  return vec3_add(r_out_perp, r_out_par);
}

real reflectance(real cosine, real ref_idx) {
  // Use Schlick's approximation for reflectance.
  real r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cosine), 5);
}

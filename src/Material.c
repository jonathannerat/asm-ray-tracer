#include <math.h>
#include <stdlib.h>

#include "Material.h"
#include "util.h"

Color emits_black(const Material *_) { return (Color){0, 0, 0}; }

spmat *lambertian_init(Color albedo) {
  Lambertian *mat = malloc(sizeof(Lambertian));
  spmat *sm = malloc(sizeof(spmat));

  mat->_material.scatter = lambertian_scatter;
  mat->_material.emitted = emits_black;
  mat->albedo = albedo;

  sm->m = (Material *)mat;
  sm->c = 0;

  return sm;
}

spmat *metal_init(Color albedo, real fuzz) {
  Metal *mat = malloc(sizeof(Metal));
  spmat *sm = malloc(sizeof(spmat));

  mat->_material.scatter = metal_scatter;
  mat->_material.emitted = emits_black;
  mat->albedo = albedo;
  mat->fuzz = fuzz;

  sm->m = (Material *)mat;
  sm->c = 0;

  return sm;
}

spmat *dielectric_init(Color albedo, real ir) {
  Dielectric *mat = malloc(sizeof(Dielectric));
  spmat *sm = malloc(sizeof(spmat));

  mat->_material.scatter = dielectric_scatter;
  mat->_material.emitted = emits_black;
  mat->albedo = albedo;
  mat->ir = ir;

  sm->m = (Material *)mat;
  sm->c = 0;

  return sm;
}

Color diffuse_light_emitted(const Material *m) { return ((DiffuseLight *)m)->albedo; }

bool diffuse_light_scatter(const Material *m, const Ray *r_in, const Record *hr, Color *attenuation,
                           Ray *scattered) {
  return false;
}

spmat *diffuse_light_init(Color albedo) {
  DiffuseLight *mat = malloc(sizeof(DiffuseLight));
  spmat *sm = malloc(sizeof(spmat));

  mat->_material.scatter = diffuse_light_scatter;
  mat->_material.emitted = diffuse_light_emitted;
  mat->albedo = albedo;

  sm->m = (Material *)mat;
  sm->c = 0;

  return sm;
}

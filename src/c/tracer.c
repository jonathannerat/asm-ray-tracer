#include "../tracer.h"
#include "../vec3.h"
#include "intersection.h"
#include "shading.h"
#include <math.h>

Color ray_color(List *, const Ray *, Color, uint);

void tracer_c(Scene *s) {
  uint width = s->output.width, height = s->output.height;
  Color bg_color = V3(0);
  Color *framebuffer = s->framebuffer;

  for (uint j = 0; j < height; j++) {
    for (uint i = 0; i < width; i++) {
      Color pixel = V3(0);

      for (uint k = 0; k < s->output.samples_per_pixel; k++) {
        real u = (i + frand()) / s->output.width;
        real v = (j + frand()) / s->output.height;
        Ray r = camera_get_ray(&s->camera, u, v);

        pixel = vec3_add(pixel, ray_color(s->world, &r, bg_color, s->output.max_depth));
      }

      framebuffer[j * width + i] = pixel;
    }
  }
}

Color ray_color(List *world, const Ray *r, Color bg, uint depth) {
  if (depth <= 0)
    return V3(0);

  HitRecord hit;

  if (!list_hit((Surface *)world, r, EPS, INFINITY, &hit))
    return bg;

  Ray scattered;
  Color attenuation;
  Color emitted = material_emitted(hit.m);

  if (!material_scatter(hit.m, r, &hit, &attenuation, &scattered))
    return emitted;

  return vec3_add(emitted,
                  vec3_prod(attenuation, ray_color(world, &scattered, bg, depth - 1)));
}

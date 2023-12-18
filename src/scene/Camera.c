#include "Camera.h"
#include <math.h>

Camera camera_new(Point from, Point to, Vec3 vup, real vfov, real aspect_ratio,
                  real aperture, real focus_dist) {
  real theta = vfov * M_PI / 180.0; // to radians
  real h = tan(theta / 2);
  real vp_height = 2.0 * h;
  real vp_width = vp_height * aspect_ratio;
  Camera c;

  c.origin = from;
  c.w = vec3_to_normalized(vec3_sub(from, to));
  c.u = vec3_to_normalized(vec3_cross(vup, c.w));
  c.v = vec3_cross(c.w, c.u);
  c.lens_radius = aperture / 2.0;
  c.horizontal = vec3_scale(c.u, focus_dist * vp_width);
  c.vertical = vec3_scale(c.v, focus_dist * vp_height);
  c.bl_corner = vec3_sub(c.origin, vec3_add(vec3_scale(c.w, focus_dist),
                                            vec3_add(vec3_scale(c.horizontal, .5),
                                                     vec3_scale(c.vertical, .5))));

  return c;
}

Ray camera_get_ray(const Camera *c, real s, real t) {
  Vec3 rand_unit = vec3_rand_unit_sphere();
  Vec3 rd = vec3_scale(rand_unit, c->lens_radius);
  Vec3 offset = vec3_add(vec3_scale(c->u, rd.x), vec3_scale(c->v, rd.y));

  Ray r = {.origin = vec3_add(c->origin, offset),
           .direction = vec3_add(vec3_add(c->bl_corner, vec3_scale(c->horizontal, s)),
                                 vec3_sub(vec3_scale(c->vertical, t), r.origin))};

  return r;
}

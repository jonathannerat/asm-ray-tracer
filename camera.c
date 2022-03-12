#include <math.h>

#include "camera.h"
#include "util.h"

camera camera_init(point from, point to, vec3 vup, double vfov,
                   double aspect_ratio, double aperture, double focus_dist) {
  double theta = degrees_to_radians(vfov);
  double h = tan(theta / 2);
  double vp_height = 2.0 * h;
  double vp_width = vp_height * aspect_ratio;
  camera c;

  c.origin = from;
  c.bl_corner = vec3_sub(c.origin, vec3_sum(3, vec3_scale(.5, c.horizontal),
                                            vec3_scale(.5, c.vertical),
                                            vec3_scale(focus_dist, c.w)));
  c.horizontal = vec3_scale(focus_dist * vp_width, c.u);
  c.vertical = vec3_scale(focus_dist * vp_height, c.v);
  c.w = vec3_normalized(vec3_sub(from, to));
  c.u = vec3_normalized(vec3_cross(vup, c.w));
  c.v = vec3_cross(c.w, c.u);
  c.lens_radius = aperture / 2.0;

  return c;
}

ray camera_get_ray(const camera *c, double s, double t) {
  vec3 rd = vec3_scale(c->lens_radius, vec3_random_in_unit_sphere());
  vec3 offset = vec3_add(vec3_scale(rd.x, c->u), vec3_scale(rd.y, c->v));

  ray r = {.origin = vec3_add(c->origin, offset),
           .direction =
               vec3_sum(4, c->bl_corner, vec3_scale(s, c->horizontal),
                        vec3_scale(t, c->vertical), vec3_inv(r.origin))};

  return r;
}

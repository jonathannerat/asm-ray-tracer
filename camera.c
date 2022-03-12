#include <math.h>

#include "camera.h"

#include "ray.h"

camera acamera_init(point from, point to, vec3 vup, float vfov,
                    float aspect_ratio, float aperture, float focus_dist) {
  float theta = degrees_to_radians(vfov);
  float h = tan(theta / 2);
  float vp_height = 2.0 * h;
  float vp_width = vp_height * aspect_ratio;

  camera c = {
      .origin = from,
      .w = vec3_normalized(vec3_sub(from, to)),
      .u = vec3_normalized(vec3_cross(vup, c.w)),
      .v = vec3_cross(c.w, c.u),
      .horizontal = vec3_scale(focus_dist * vp_width, c.u),
      .vertical = vec3_scale(focus_dist * vp_height, c.v),
      .bl_corner = vec3_sub(c.origin, vec3_sum(3, vec3_scale(.5, c.horizontal),
                                               vec3_scale(.5, c.vertical),
                                               vec3_scale(focus_dist, c.w))),
      .lens_radius = aperture / 2.0,
  };

  return c;
}

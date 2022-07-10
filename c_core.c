#include "core.h"
#include "hittable/Box.h"
#include "hittable/List.h"

bool box_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr) {
  Box *self = (Box *)_self;
  uint i;
  bool hit_anything = false;
  double closest_so_far = t_max;
  Record tmp;

  for (i = 0; i < self->faces->size; i++) {
    Hittable *h = list_get(self->faces, i);
    if (h->hit(h, r, t_min, closest_so_far, &tmp) && box_is_inside(self, tmp.p)) {
      hit_anything = true;
      closest_so_far = tmp.t;
      *hr = tmp;
    }
  }

  return hit_anything;
}

bool list_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr) {
  List *self = (List *)_self;
  Record tmp;
  uint i;
  bool hit_anything = false;
  double closest_so_far = t_max;

  for (i = 0; i < self->size; i++) {
    Hittable *h = self->list[i];
    if (h->hit(h, r, t_min, closest_so_far, &tmp)) {
      hit_anything = true;
      closest_so_far = tmp.t;
      *hr = tmp;
    }
  }

  return hit_anything;
}

vec3 vec3_add(const vec3 a, const vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 vec3_inv(const vec3 v) { return (vec3){-v.x, -v.y, -v.z}; }

vec3 vec3_prod(const vec3 a, const vec3 b) {
  return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

vec3 vec3_scale(double s, const vec3 v) {
  return (vec3){v.x * s, v.y * s, v.z * s};
}

vec3 vec3_unscale(const vec3 v, double s) {
  return (vec3){v.x / s, v.y / s, v.z / s};
}

vec3 vec3_sub(const vec3 a, const vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

double vec3_norm2(const vec3 v) {
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

vec3 cross(const vec3 a, const vec3 b) {
  return (vec3){
      a.y * b.z - b.y * a.z,
      b.x * a.z - a.x * b.z,
      a.x * b.y - b.x * a.y,
  };
}

double dot(const vec3 a, const vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

bool perpendicular(const vec3 a, const vec3 b) {
  return fabs(dot(a, b)) < EPS;
}

vec3 normalized(const vec3 v) {
  return vec3_scale(1.0 / sqrt(vec3_norm2(v)), v);
}

bool vec3_near_zero(const vec3 v) {
  return fabs(v.x) < EPS && fabs(v.y) < EPS && fabs(v.z) < EPS ;
}

vec3 reflect(const vec3 v, const vec3 n) {
  return vec3_sub(v, vec3_scale(2 * dot(v, n), n));
}

vec3 refract(const vec3 uv, const vec3 n, double etai_over_etat) {
  double cos_theta = fmin(dot(vec3_inv(uv), n), 1.0);
  vec3 r_out_perp = vec3_scale(etai_over_etat, vec3_add(uv, vec3_scale(cos_theta, n)));
  vec3 r_out_par = vec3_scale(-sqrt(fabs(1.0 - vec3_norm2(r_out_perp))), n);
  return vec3_add(r_out_perp, r_out_par);
}

vec3 vec3_random_in_unit_sphere() {
  while (1) {
    vec3 p = vec3_random_between(-1, 1);

    if (vec3_norm2(p) >= 1)
      continue;

    return p;
  }
}

vec3 vec3_random() {
  return (vec3){random_double(), random_double(), random_double()};
}

vec3 vec3_random_between(double min, double max) {
  return (vec3){random_double_between(min, max),
                random_double_between(min, max),
                random_double_between(min, max)};
}

point ray_at(const Ray *r, double t) {
  return vec3_add(r->origin, vec3_scale(t, r->direction));
}

Camera camera_init(point from, point to, vec3 vup, double vfov, double aspect_ratio,
                   double aperture, double focus_dist) {
  double theta = degrees_to_radians(vfov);
  double h = tan(theta / 2);
  double vp_height = 2.0 * h;
  double vp_width = vp_height * aspect_ratio;
  Camera c;

  c.origin = from;
  c.w = normalized(vec3_sub(from, to));
  c.u = normalized(cross(vup, c.w));
  c.v = cross(c.w, c.u);
  c.lens_radius = aperture / 2.0;
  c.horizontal = vec3_scale(focus_dist * vp_width, c.u);
  c.vertical = vec3_scale(focus_dist * vp_height, c.v);
  c.bl_corner =
    vec3_sub(c.origin, vec3_add(vec3_scale(focus_dist, c.w), vec3_add(vec3_scale(.5, c.horizontal),
                                                                      vec3_scale(.5, c.vertical))));

  return c;
}

Ray camera_get_ray(const Camera *c, double s, double t) {
  vec3 rand_unit = vec3_random_in_unit_sphere();
  vec3 rd = vec3_scale(c->lens_radius, rand_unit);
  vec3 offset = vec3_add(vec3_scale(rd.x, c->u), vec3_scale(rd.y, c->v));

  Ray r = {.origin = vec3_add(c->origin, offset),
           .direction = vec3_add(vec3_add(c->bl_corner, vec3_scale(s, c->horizontal)),
                                 vec3_add(vec3_scale(t, c->vertical), vec3_inv(r.origin)))};

  return r;
}

void hr_set_face_normal(Record *hr, const Ray *r, vec3 n) {
  hr->front_face = dot(r->direction, n) < 0;
  hr->normal = hr->front_face ? n : vec3_inv(n);
}


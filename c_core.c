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

bool plane_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr) {
  Plane *self = (Plane *)_self;

  if (perpendicular(r->direction, self->normal))
    return false;

  double t = dot(vec3_sub(self->origin, r->origin), self->normal) / dot(r->direction, self->normal);

  if (t < t_min || t_max < t)
    return false;

  hr->t = t;
  hr->p = ray_at(r, t);
  hr->sm = self->sm;
  hr_set_face_normal(hr, r, self->normal);

  return true;
}

bool sphere_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr) {
  Sphere *self = (Sphere *)_self;

  Vec3 oc = vec3_sub(r->origin, self->center);
  double a = vec3_norm2(r->direction);
  double hb = dot(oc, r->direction);
  double c = vec3_norm2(oc) - self->radius * self->radius;
  double discriminant = hb * hb - a * c;

  if (discriminant < 0)
    return false;

  double sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  double root = (-hb - sqrtd) / a;
  if (root < t_min || t_max < root) {
    root = (-hb + sqrtd) / a;
    if (root < t_min || t_max < root)
      return false;
  }

  hr->t = root;
  hr->p = ray_at(r, hr->t);
  hr->sm = self->sm;
  Vec3 outward_normal = vec3_scale(1 / self->radius, vec3_sub(hr->p, self->center));
  hr_set_face_normal(hr, r, outward_normal);

  return true;
}

bool triangle_hit(const Hittable *_self, const Ray *r, double t_min, double t_max, Record *hr) {
  Triangle *self = (Triangle *)_self;

  Vec3 normal = cross(vec3_sub(self->p2, self->p1), vec3_sub(self->p3, self->p1));

  if (perpendicular(normal, r->direction))
    return false;

  double t = dot(vec3_sub(self->p1, r->origin), normal) / dot(r->direction, normal);
  if (t < t_min || t_max < t)
    return false;

  Point p = ray_at(r, t);

  if (dot(normal, cross(vec3_sub(self->p2, self->p1), vec3_sub(p, self->p1))) > 0 &&
      dot(normal, cross(vec3_sub(self->p3, self->p2), vec3_sub(p, self->p2))) > 0 &&
      dot(normal, cross(vec3_sub(self->p1, self->p3), vec3_sub(p, self->p3))) > 0) {
    hr->t = t;
    hr->p = p;
    hr->sm = self->sm;
    hr_set_face_normal(hr, r, normal);
    return true;
  }

  return false;
}

Vec3 vec3_add(const Vec3 a, const Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }

Vec3 vec3_inv(const Vec3 v) { return (Vec3){-v.x, -v.y, -v.z}; }

Vec3 vec3_prod(const Vec3 a, const Vec3 b) { return (Vec3){a.x * b.x, a.y * b.y, a.z * b.z}; }

Vec3 vec3_scale(double s, const Vec3 v) { return (Vec3){v.x * s, v.y * s, v.z * s}; }

Vec3 vec3_unscale(const Vec3 v, double s) { return (Vec3){v.x / s, v.y / s, v.z / s}; }

Vec3 vec3_sub(const Vec3 a, const Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }

double vec3_norm2(const Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

Vec3 cross(const Vec3 a, const Vec3 b) {
  return (Vec3){
    a.y * b.z - b.y * a.z,
    b.x * a.z - a.x * b.z,
    a.x * b.y - b.x * a.y,
  };
}

double dot(const Vec3 a, const Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

bool perpendicular(const Vec3 a, const Vec3 b) { return fabs(dot(a, b)) < EPS; }

Vec3 normalized(const Vec3 v) { return vec3_scale(1.0 / sqrt(vec3_norm2(v)), v); }

bool vec3_near_zero(const Vec3 v) { return fabs(v.x) < EPS && fabs(v.y) < EPS && fabs(v.z) < EPS; }

Vec3 reflect(const Vec3 v, const Vec3 n) { return vec3_sub(v, vec3_scale(2 * dot(v, n), n)); }

Vec3 refract(const Vec3 uv, const Vec3 n, double etai_over_etat) {
  double cos_theta = fmin(dot(vec3_inv(uv), n), 1.0);
  Vec3 r_out_perp = vec3_scale(etai_over_etat, vec3_add(uv, vec3_scale(cos_theta, n)));
  Vec3 r_out_par = vec3_scale(-sqrt(fabs(1.0 - vec3_norm2(r_out_perp))), n);
  return vec3_add(r_out_perp, r_out_par);
}

Vec3 vec3_random_in_unit_sphere() {
  while (1) {
    Vec3 p = vec3_random_between(-1, 1);

    if (vec3_norm2(p) >= 1)
      continue;

    return p;
  }
}

Vec3 vec3_random() { return (Vec3){random_double(), random_double(), random_double()}; }

Vec3 vec3_random_between(double min, double max) {
  return (Vec3){random_double_between(min, max), random_double_between(min, max),
                random_double_between(min, max)};
}

Point ray_at(const Ray *r, double t) { return vec3_add(r->origin, vec3_scale(t, r->direction)); }

Camera camera_init(Point from, Point to, Vec3 vup, double vfov, double aspect_ratio,
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
  Vec3 rand_unit = vec3_random_in_unit_sphere();
  Vec3 rd = vec3_scale(c->lens_radius, rand_unit);
  Vec3 offset = vec3_add(vec3_scale(rd.x, c->u), vec3_scale(rd.y, c->v));

  Ray r = {.origin = vec3_add(c->origin, offset),
           .direction = vec3_add(vec3_add(c->bl_corner, vec3_scale(s, c->horizontal)),
                                 vec3_add(vec3_scale(t, c->vertical), vec3_inv(r.origin)))};

  return r;
}

void hr_set_face_normal(Record *hr, const Ray *r, Vec3 n) {
  hr->front_face = dot(r->direction, n) < 0;
  hr->normal = hr->front_face ? n : vec3_inv(n);
}

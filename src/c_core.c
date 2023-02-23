#include "core.h"
#include "hittable/Box.h"
#include "hittable/List.h"
#include "util.h"
#include <stdio.h>

Vec3 cross(const Vec3 a, const Vec3 b);
real dot(const Vec3 a, const Vec3 b);
bool perpendicular(const Vec3 a, const Vec3 b);
Vec3 normalized(const Vec3 v);
bool vec3_near_zero(const Vec3 v);
Vec3 refract(const Vec3 uv, const Vec3 n, real etai_over_etat);
Vec3 reflect(const Vec3 v, const Vec3 n);
real reflectance(real cosine, real ref_idx);
void hr_set_face_normal(Record *hr, const Ray *r, Vec3 n);

// Declared in header

Vec3 vec3_add(const Vec3 a, const Vec3 b) { return V(a.x + b.x, a.y + b.y, a.z + b.z); }

Vec3 vec3_inv(const Vec3 v) { return V(-v.x, -v.y, -v.z); }

Vec3 vec3_prod(const Vec3 a, const Vec3 b) { return V(a.x * b.x, a.y * b.y, a.z * b.z); }

Vec3 vec3_scale(real s, const Vec3 v) { return V(v.x * s, v.y * s, v.z * s); }

Vec3 vec3_unscale(const Vec3 v, real s) { return V(v.x / s, v.y / s, v.z / s); }

Vec3 vec3_sub(const Vec3 a, const Vec3 b) { return V(a.x - b.x, a.y - b.y, a.z - b.z); }

real vec3_norm2(const Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

bool box_hit(const Hittable *_self, const Ray *r, real t_min, real t_max, Record *hr) {
  Box *self = (Box *)_self;
  uint i;
  bool hit_anything = false;
  real closest_so_far = t_max;
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

bool plane_hit(const Hittable *_self, const Ray *r, real t_min, real t_max, Record *hr) {
  Plane *self = (Plane *)_self;

  if (perpendicular(r->direction, self->normal))
    return false;

  real t = dot(vec3_sub(self->origin, r->origin), self->normal) / dot(r->direction, self->normal);

  if (t < t_min || t_max < t)
    return false;

  hr->t = t;
  hr->p = ray_at(r, t);
  hr->sm = self->sm;
  hr_set_face_normal(hr, r, self->normal);

  return true;
}

bool sphere_hit(const Hittable *_self, const Ray *r, real t_min, real t_max, Record *hr) {
  Sphere *self = (Sphere *)_self;

  Vec3 oc = vec3_sub(r->origin, self->center);
  real a = vec3_norm2(r->direction);
  real hb = dot(oc, r->direction);
  real c = vec3_norm2(oc) - self->radius * self->radius;
  real discriminant = hb * hb - a * c;

  if (discriminant < 0)
    return false;

  real sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  real root = (-hb - sqrtd) / a;
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

bool triangle_hit(const Hittable *_self, const Ray *r, real t_min, real t_max, Record *hr) {
  Triangle *self = (Triangle *)_self;

  Vec3 normal = cross(vec3_sub(self->p2, self->p1), vec3_sub(self->p3, self->p1));

  if (perpendicular(normal, r->direction))
    return false;

  real t = dot(vec3_sub(self->p1, r->origin), normal) / dot(r->direction, normal);
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

bool lambertian_scatter(const Material *m, const Ray *r_in, const Record *hr, Color *attenuation,
                        Ray *scattered) {
  Lambertian *self = (Lambertian *)m;
  Vec3 scatter_direction = vec3_add(hr->normal, vec3_rnd_unit_sphere());

  if (vec3_near_zero(scatter_direction))
    scatter_direction = hr->normal;

  *scattered = (Ray){hr->p, scatter_direction};
  *attenuation = self->albedo;
  return true;
}

bool metal_scatter(const Material *m, const Ray *r_in, const Record *hr, Color *attenuation,
                   Ray *scattered) {
  Metal *self = (Metal *)m;
  Vec3 reflected = reflect(normalized(r_in->direction), hr->normal);

  *scattered = (Ray){
    hr->p,
    vec3_add(reflected, vec3_scale(self->fuzz, vec3_rnd_unit_sphere())),
  };
  *attenuation = self->albedo;

  return dot(reflected, hr->normal) > 0;
}

bool dielectric_scatter(const Material *m, const Ray *r_in, const Record *hr, Color *attenuation,
                        Ray *scattered) {
  Dielectric *self = (Dielectric *)m;
  real ref_ratio = hr->front_face ? 1 / self->ir : self->ir;
  Vec3 unit_dir = normalized(r_in->direction);
  real cos_theta = fmin(dot(vec3_inv(unit_dir), hr->normal), 1);
  real sin_theta = sqrt(1 - cos_theta * cos_theta);
  bool cannot_refract = ref_ratio * sin_theta > 1;
  Vec3 direction;

  if (cannot_refract || reflectance(cos_theta, ref_ratio) > rnd())
    direction = reflect(unit_dir, hr->normal);
  else
    direction = refract(unit_dir, hr->normal, ref_ratio);

  *attenuation = self->albedo;
  *scattered = (Ray){hr->p, direction};

  return true;
}

Camera camera_init(Point from, Point to, Vec3 vup, real vfov, real aspect_ratio, real aperture,
                   real focus_dist) {
  real theta = vfov * M_PI / 180.0; // to radians
  real h = tan(theta / 2);
  real vp_height = 2.0 * h;
  real vp_width = vp_height * aspect_ratio;
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

Ray camera_get_ray(const Camera *c, real s, real t) {
  Vec3 rand_unit = vec3_rnd_unit_sphere();
  Vec3 rd = vec3_scale(c->lens_radius, rand_unit);
  Vec3 offset = vec3_add(vec3_scale(rd.x, c->u), vec3_scale(rd.y, c->v));

  Ray r = {.origin = vec3_add(c->origin, offset),
           .direction = vec3_add(vec3_add(c->bl_corner, vec3_scale(s, c->horizontal)),
                                 vec3_sub(vec3_scale(t, c->vertical), r.origin))};

  return r;
}

void scene_render(const Scene *s, Color *image) {
  int i, j, k, w = s->output.width, h = s->output.height;
  Color bg_color = {0, 0, 0};

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      Color pixel = {0, 0, 0};

      for (k = 0; k < s->output.samples_per_pixel; k++) {
        real u = (i + rnd()) / (s->output.width - 1);
        real v = (j + rnd()) / (s->output.height - 1);
        Ray r = camera_get_ray(&s->camera, u, v);
        pixel = vec3_add(pixel, ray_color(s->world, &r, bg_color, s->output.max_depth));
      }

      image[j * w + i] = pixel;
    }
  }
}

Color ray_color(Hittable *world, const Ray *r, Color bg, uint depth) {
  if (depth <= 0)
    return (Color){0, 0, 0};

  Record rec;

  if (!world->hit(world, r, EPS, INFINITY, &rec))
    return bg;

  Material *m = rec.sm ? rec.sm->m : NULL;
  Ray scattered;
  Color attenuation;
  Color emitted = m->emitted(m);

  if (!m->scatter(m, r, &rec, &attenuation, &scattered))
    return emitted;

  return vec3_add(emitted, vec3_prod(attenuation, ray_color(world, &scattered, bg, depth - 1)));
}

// Internal

void hr_set_face_normal(Record *hr, const Ray *r, Vec3 n) {
  hr->front_face = dot(r->direction, n) < 0;
  hr->normal = hr->front_face ? n : vec3_inv(n);
}

Vec3 cross(const Vec3 a, const Vec3 b) {
  return V(a.y * b.z - b.y * a.z, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y);
}

real dot(const Vec3 a, const Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

bool perpendicular(const Vec3 a, const Vec3 b) { return fabs(dot(a, b)) < EPS; }

Vec3 normalized(const Vec3 v) { return vec3_scale(1.0 / sqrt(vec3_norm2(v)), v); }

bool vec3_near_zero(const Vec3 v) { return fabs(v.x) < EPS && fabs(v.y) < EPS && fabs(v.z) < EPS; }

Vec3 refract(const Vec3 uv, const Vec3 n, real etai_over_etat) {
  real cos_theta = fmin(dot(vec3_inv(uv), n), 1.0);
  Vec3 r_out_perp = vec3_scale(etai_over_etat, vec3_add(uv, vec3_scale(cos_theta, n)));
  Vec3 r_out_par = vec3_scale(-sqrt(fabs(1.0 - vec3_norm2(r_out_perp))), n);
  return vec3_add(r_out_perp, r_out_par);
}

Vec3 reflect(const Vec3 v, const Vec3 n) { return vec3_sub(v, vec3_scale(2 * dot(v, n), n)); }

real reflectance(real cosine, real ref_idx) {
  // Use Schlick's approximation for reflectance.
  real r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cosine), 5);
}

Vec3 vec3_rnd_unit_sphere() {
  while (1) {
    Vec3 p = V(rnd() * 2 - 1, rnd() * 2 - 1, rnd() * 2 - 1);

    if (vec3_norm2(p) >= 1)
      continue;

    return p;
  }
}

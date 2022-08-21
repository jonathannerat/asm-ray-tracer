#include <hittable/Plane.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>

#include "Material.h"
#include "core.h"
#include "hittable/Box.h"
#include "hittable/Sphere.h"
#include "hittable/Triangle.h"
#include "util.h"

typedef enum {
  PLANE,
  BOX,
  TRIANGLE,
  SPHERE,
} HittableType;

typedef enum { LAMBERTIAN, METAL, DIELECTRIC, LIGHT } MaterialType;

Hittable *get_hittable_in_ray_range_with_material(HittableType type, const Ray *r, spmat *sm);
Hittable *get_hittable_in_ray_range(HittableType type, const Ray *r);
double measure_avg_hit_time(uint times, HittableType type);
double measure_avg_scatter_time(uint times, MaterialType type);

double diffusec(struct timeval *start, struct timeval *end) {
  return (end->tv_sec - start->tv_sec) * 1000000.0 + (end->tv_usec - start->tv_usec);
}

int main(int argc, char **argv) {
  // repetitions
  uint n = argc > 1 ? strtoul(argv[1], NULL, 10) : 1000;

  fprintf(stderr, "%s:\n", argv[0]);
  fprintf(stderr, "====================\n");
  fprintf(stderr, "Avg. HIT times:\n");
  fprintf(stderr, "plane:      %.4f us\n", measure_avg_hit_time(n, PLANE));
  fprintf(stderr, "box:        %.4f us\n", measure_avg_hit_time(n, BOX));
  fprintf(stderr, "triangle:   %.4f us\n", measure_avg_hit_time(n, TRIANGLE));
  fprintf(stderr, "sphere:     %.4f us\n", measure_avg_hit_time(n, SPHERE));
  fprintf(stderr, "====================\n");

  fprintf(stderr, "====================\n");
  fprintf(stderr, "Avg. SCATTER times:\n");
  fprintf(stderr, "lambertian:    %.4f us\n", measure_avg_scatter_time(n, LAMBERTIAN));
  fprintf(stderr, "metal:         %.4f us\n", measure_avg_scatter_time(n, METAL));
  fprintf(stderr, "dielectric:    %.4f us\n", measure_avg_scatter_time(n, DIELECTRIC));
  fprintf(stderr, "light:         %.4f us\n", measure_avg_scatter_time(n, LIGHT));
  fprintf(stderr, "====================\n");

  return 0;
}

Hittable *get_hittable_in_ray_range_with_material(HittableType type, const Ray *r, spmat *sm) {
  Hittable *h = NULL;
  Vec3 p = ray_at(r, rnd() * 10 + 1);

  if (type == PLANE) {
    h = plane_init(p, vec3_sub(V(0, 0, 0), r->direction), sm);
  } else if (type == BOX) {
    h = box_init(vec3_sub(p, V(.5, .5, .5)), vec3_add(p, V(.5, .5, .5)), sm);
  } else if (type == TRIANGLE) {
    real ox = rnd() + .1;
    real oy = rnd() + .1;
    real oz = (p.x * ox + p.y * oy) / (-p.z);
    Vec3 q = V(ox, oy, oz);
    // r = p x q
    Vec3 r = V(p.y * q.z - q.y * p.z, q.x * p.z - p.x * q.z, p.x * q.y - q.x * p.y);
    h = triangle_init(vec3_add(p, q), vec3_add(p, r), vec3_sub(p, vec3_add(q, r)), sm);
  } else if (type == SPHERE) {
    h = sphere_init(p, rnd() + .1, sm);
  }

  return h;
}

Hittable *get_hittable_in_ray_range(HittableType type, const Ray *r) {
  return get_hittable_in_ray_range_with_material(type, r, NULL);
}

double measure_avg_hit_time(uint times, HittableType type) {
  uint i;
  double total_usec = 0;
  Ray r = {.origin = V(0, 0, 0)};
  Record tmp;
  Hittable *h;
  struct timeval start, end;

  for (i = 0; i < times; i++) {
    r.direction = vec3_rnd_unit_sphere();
    h = get_hittable_in_ray_range(type, &r);

    gettimeofday(&start, NULL);
    HIT(h, &r, EPS, INFINITY, &tmp);
    gettimeofday(&end, NULL);

    DESTROY(h);

    total_usec += diffusec(&start, &end);
  }

  return total_usec / times;
}

double measure_avg_scatter_time(uint times, MaterialType type) {
  uint i;
  double total_usec = 0;
  real rnd_hittable;
  Ray r = {.origin = V(0, 0, 0)}, scattered;
  Record tmp;
  Hittable *h;
  spmat *m;
  struct timeval start, end;
  Color c = vec3_unscale(vec3_add(vec3_rnd_unit_sphere(), V(1, 1, 1)), 2), attenuation;

  for (i = 0; i < times; i++) {
    rnd_hittable = rnd();
    r.direction = vec3_rnd_unit_sphere();
    m = type == LAMBERTIAN   ? lambertian_init(c)
        : type == METAL      ? metal_init(c, rnd())
        : type == DIELECTRIC ? dielectric_init(c, rnd())
                             : diffuse_light_init(c);
    HittableType htype = type == DIELECTRIC   ? (rnd_hittable < .5 ? SPHERE : BOX)
                         : rnd_hittable < .25 ? PLANE
                         : rnd_hittable < .5  ? BOX
                         : rnd_hittable < .75 ? TRIANGLE
                                              : SPHERE;
    h = get_hittable_in_ray_range_with_material(htype, &r, m);

    HIT(h, &r, EPS, INFINITY, &tmp);
    gettimeofday(&start, NULL);
    SCATTER(tmp.sm->m, &r, &tmp, &attenuation, &scattered);
    gettimeofday(&end, NULL);

    DESTROY(h);

    total_usec += diffusec(&start, &end);
  }

  return total_usec / times;
}

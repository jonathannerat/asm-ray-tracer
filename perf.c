#include <hittable/Plane.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>

#include "core.h"
#include "hittable/Box.h"
#include "hittable/Triangle.h"
#include "hittable/Sphere.h"
#include "util.h"

double diffusec(struct timeval *start, struct timeval *end) {
  return (end->tv_sec - start->tv_sec) * 1000000.0 + (end->tv_usec - start->tv_usec);
}

int main(int argc, char **argv) {
  uint n = argc > 1 ? strtoul(argv[1], NULL, 10) : 10000, i;

  // BOX
  Ray r = {V(.5, .5, 2), V(0, 0, -1)};
  Hittable *b = box_init(V(0, 0, 0), V(1, 1, 1), NULL);
  Record tmp;
  struct timeval start, end;

  gettimeofday(&start, NULL);
  for (i = 0; i < n; i++) {
    HIT(b, &r, 0, INFINITY, &tmp);
  }
  DESTROY(b);
  gettimeofday(&end, NULL);
  printf("box_hit avg time (us): %.4f\n", diffusec(&start, &end) / n);

  // PLANE
  r.direction = V(0, -1, -1);
  b = plane_init(V(0,0,0), V(0,1,0), NULL);

  gettimeofday(&start, NULL);
  for (i = 0; i < n; i++) {
    HIT(b, &r, 0, INFINITY, &tmp);
  }
  DESTROY(b);
  gettimeofday(&end, NULL);
  printf("plane_hit avg time (us): %.4f\n", diffusec(&start, &end) / n);

  // TRIANGLE
  r.direction = V(0, -.5, -.2);
  b = triangle_init(V(0,-1,-1), V(1,-1,-1), V(.5,.5,0), NULL);

  gettimeofday(&start, NULL);
  for (i = 0; i < n; i++) {
    HIT(b, &r, 0, INFINITY, &tmp);
  }
  DESTROY(b);
  gettimeofday(&end, NULL);
  printf("triangle_hit avg time (us): %.4f\n", diffusec(&start, &end) / n);

  // SPHERE
  r.direction = V(0, -.5, -.2);
  b = sphere_init(V(.5,0,0), .8, NULL);

  gettimeofday(&start, NULL);
  for (i = 0; i < n; i++) {
    HIT(b, &r, 0, INFINITY, &tmp);
  }
  DESTROY(b);
  gettimeofday(&end, NULL);
  printf("sphere_hit avg time (us): %.4f\n", diffusec(&start, &end) / n);

  return 0;
}

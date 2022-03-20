#include "Triangle.h"

bool_ triangle_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);

Triangle *triangle_init(point p1, point p2, point p3) {
  Triangle *self = malloc(sizeof(Triangle));

  self->_hittable.hit = triangle_hit;

  self->p1 = p1;
  self->p2 = p2;
  self->p3 = p3;

  return self;
}

bool_ triangle_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
  Triangle *self = (Triangle *)_self;

  vec3 normal = cross(vec3_sub(self->p2, self->p1), vec3_sub(self->p3, self->p1));

  if (perpendicular(normal, r->direction))
    return false;

  double t = dot(vec3_sub(self->p1, r->origin), normal) / dot(r->direction, normal);
  if (t < t_min || t_max < t)
    return false;

  point p = ray_at(r, t);

  if (dot(normal, cross(vec3_sub(self->p2, self->p1), vec3_sub(p, self->p1))) > 0 &&
      dot(normal, cross(vec3_sub(self->p3, self->p2), vec3_sub(p, self->p2))) > 0 &&
      dot(normal, cross(vec3_sub(self->p1, self->p3), vec3_sub(p, self->p3))) > 0) {
    hr->t = t;
    hr->p = p;
    hr_set_face_normal(hr, r, normal);
    return true;
  }

  return false;
}

void triangle_destroy(Triangle *t) { free(t); }

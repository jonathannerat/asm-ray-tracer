#include "Triangle.h"

bool triangle_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr);
void triangle_destroy(Hittable *h);
Box *triangle_bbox(const Hittable *h);

Hittable *triangle_init(point p1, point p2, point p3, shrmat sm) {
  Triangle *self = malloc(sizeof(Triangle));

  self->_hittable.hit = triangle_hit;
  self->_hittable.destroy = triangle_destroy;
  self->_hittable.bbox = triangle_bbox;

  self->p1 = p1;
  self->p2 = p2;
  self->p3 = p3;

  point cback = {MIN(MIN(p1.x, p2.x), p3.x), MIN(MIN(p1.y, p2.y), p3.y),
                 MIN(MIN(p1.z, p2.z), p3.z)};
  point cfront = {MAX(MAX(p1.x, p2.x), p3.x), MAX(MAX(p1.y, p2.y), p3.y),
                  MAX(MAX(p1.z, p2.z), p3.z)};

  self->bbox = (Box *)box_init(cback, cfront, (shrmat){.m = NULL});

  sm.refcount++;
  self->sm = sm;

  return (Hittable *)self;
}

bool triangle_hit(const Hittable *_self, const ray *r, double t_min, double t_max, Record *hr) {
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
    hr->mat = self->sm.m;
    hr_set_face_normal(hr, r, normal);
    return true;
  }

  return false;
}

void triangle_destroy(Hittable *h) {
  Triangle *self = (Triangle *)h;

  if (!--self->sm.refcount && self->sm.m)
    free(self->sm.m);

  DESTROY(self->bbox);

  free(h);
}

Box *triangle_bbox(const Hittable *h) { return ((Triangle *)h)->bbox; }

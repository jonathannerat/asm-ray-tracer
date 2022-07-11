// vi: fdm=marker
#ifndef CORE_H
#define CORE_H

typedef unsigned int uint;
typedef double real;

typedef int bool;
#define true 1
#define false 0


// TYPES {{{

// Vectors and Rays {{{

typedef struct {
  real x;
  real y;
  real z;
  real _; // padding
} Vec3;

typedef Vec3 Point;
typedef Vec3 Color;

typedef struct {
  Point origin;
  Vec3 direction;
} Ray;

// }}}

// Materials {{{
struct _material;

struct _spmat {
  struct _material *m;
  uint c;
};

struct _record {
  real t;
  Point p;
  Vec3 normal;
  bool front_face;
  struct _spmat *sm;
};

struct _material {
  bool (*scatter)(const struct _material *m, const Ray *r_in, const struct _record *hr,
                  Color *attenuation, Ray *scattered);
  Color (*emitted)(const struct _material *m);
};

struct _lambertian {
  struct _material _material;
  Color albedo;
};

struct _metal {
  struct _material _material;
  Color albedo;
  real fuzz;
};

struct _dielectric {
  struct _material _material;
  Color albedo;
  real ir;
};

struct _diffuse_light {
  struct _material _material;
  Color albedo;
};

typedef struct _spmat spmat;
typedef struct _record Record;
typedef struct _material Material;
typedef struct _lambertian Lambertian;
typedef struct _metal Metal;
typedef struct _dielectric Dielectric;
typedef struct _diffuse_light DiffuseLight;

// }}}

// Hittables {{{

struct _hittable {
  bool (*hit)(const struct _hittable *o, const Ray *r, real t_min, real t_max,
              struct _record *hr);

  void (*destroy)(struct _hittable *);

  struct _box *(*bbox)(const struct _hittable *);

  Point refp;
};

struct _list {
  struct _hittable _hittable;
  struct _hittable **list;
  uint size;
  uint cap;
  Point refpsum;
  spmat *sm;
  struct _box *bbox;
};

struct _box {
  struct _hittable _hittable;
  Point cback;
  Point cfront;
  struct _list *faces;
  spmat *sm;
};

struct _plane {
  struct _hittable _hittable;
  Point origin;
  Vec3 normal;
  spmat *sm;
};

struct _sphere {
  struct _hittable _hittable;
  Point center;
  real radius;
  struct _box *bbox;
  spmat *sm;
};

struct _triangle {
  struct _hittable _hittable;
  Point p1, p2, p3;
  struct _box *bbox;
  spmat *sm;
};

struct _node {
  struct _hittable _hittable;
  struct _node *left, *right;
  struct _box *bbox;
  struct _list *objects;
};

struct _kdtree {
  struct _hittable _hittable;
  struct _node *root;
};

typedef struct _box Box;
typedef struct _list List;
typedef struct _hittable Hittable;
typedef struct _plane Plane;
typedef struct _sphere Sphere;
typedef struct _triangle Triangle;
typedef struct _node Node;
typedef struct _kdtree KDTree;

// }}}

// Camera {{{

struct _camera {
  Point origin;
  Point bl_corner;
  Vec3 horizontal;
  Vec3 vertical;
  Vec3 u, v, w;
  real lens_radius;
};

typedef struct _camera Camera;

// }}}

// }}}

// Vec3 methods {{{

Vec3 vec3_add(const Vec3 a, const Vec3 b);
Vec3 vec3_inv(const Vec3 v);
Vec3 vec3_prod(const Vec3 a, const Vec3 b);
Vec3 vec3_unscale(const Vec3 v, real s);
Vec3 vec3_sub(const Vec3 a, const Vec3 b);
real vec3_norm2(const Vec3 a);

// }}}

// Object hit methods {{{

bool box_hit(const Hittable *_self, const Ray *ray, real t_min, real t_max, Record *hr);
bool plane_hit(const Hittable *_self, const Ray *r, real t_min, real t_max, Record *hr);
bool sphere_hit(const Hittable *_self, const Ray *r, real t_min, real t_max, Record *hr);
bool triangle_hit(const Hittable *_self, const Ray *r, real t_min, real t_max, Record *hr);

// }}}

// Material scatter methods {{{

bool lambertian_scatter(const Material *m, const Ray *r_in, const Record *hr, Color *attenuation,
                        Ray *scattered);
bool metal_scatter(const Material *m, const Ray *r_in, const Record *hr, Color *attenuation,
                   Ray *scattered);
bool dielectric_scatter(const Material *m, const Ray *r_in, const Record *hr, Color *attenuation,
                        Ray *scattered);

// }}}

// Other {{{

void hr_set_face_normal(Record *hr, const Ray *r, Vec3 n);
Camera camera_init(Point from, Point to, Vec3 vup, real vfov, real aspect_ratio,
                   real aperture, real focus_dist);
Ray camera_get_ray(const Camera *c, real s, real t);

// }}}

#endif // CORE_H

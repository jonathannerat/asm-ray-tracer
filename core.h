#ifndef CORE_H
#define CORE_H

typedef unsigned int uint;
typedef int bool;
#define true 1
#define false 0

// Vectors and Rays

typedef struct {
  double x;
  double y;
  double z;
} vec3;

typedef vec3 point;
typedef vec3 color;

typedef struct {
  point origin;
  vec3 direction;
} Ray;

// Materials
struct _material;

struct _spmat {
  struct _material *m;
  uint c;
};

struct _record {
  double t;
  point p;
  vec3 normal;
  bool front_face;
  struct _spmat *sm;
};

struct _material {
  bool (*scatter)(const struct _material *m, const Ray *r_in, const struct _record *hr,
                  color *attenuation, Ray *scattered);
  color (*emitted)(const struct _material *m);
};

typedef struct _spmat spmat;
typedef struct _record Record;
typedef struct _material Material;

// Hittables

struct _hittable {
  bool (*hit)(const struct _hittable *o, const Ray *r, double t_min, double t_max,
              struct _record *hr);

  void (*destroy)(struct _hittable *);

  struct _box *(*bbox)(const struct _hittable *);

  point refp;
};

struct _list {
  struct _hittable _hittable;
  struct _hittable **list;
  uint size;
  uint cap;
  point refpsum;
  spmat *sm;
  struct _box *bbox;
};

struct _box {
  struct _hittable _hittable;
  point cback;
  point cfront;
  struct _list *faces;
  spmat *sm;
};

struct _plane {
  struct _hittable _hittable;
  point origin;
  vec3 normal;
  spmat *sm;
};

struct _sphere {
  struct _hittable _hittable;
  point center;
  double radius;
  struct _box *bbox;
  spmat *sm;
};

struct _triangle {
  struct _hittable _hittable;
  point p1, p2, p3;
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

// Camera

struct _camera {
  point origin;
  point bl_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
  double lens_radius;
};

typedef struct _camera Camera;

vec3 vec3_add(const vec3 a, const vec3 b);
vec3 vec3_inv(const vec3 v);
vec3 vec3_prod(const vec3 a, const vec3 b);
vec3 vec3_scale(double s, const vec3 v);
vec3 vec3_unscale(const vec3 v, double s);
vec3 vec3_sub(const vec3 a, const vec3 b);
vec3 cross(const vec3 a, const vec3 b);
double dot(const vec3 a, const vec3 b);
double vec3_norm2(const vec3 a);
bool perpendicular(const vec3 a, const vec3 b);
vec3 normalized(const vec3 v);
bool vec3_near_zero(const vec3 v);
vec3 reflect(const vec3 v, const vec3 n);
vec3 refract(const vec3 uv, const vec3 n, double etai_over_etat);
vec3 vec3_random_in_unit_sphere();
vec3 vec3_random();
vec3 vec3_random_between(double min, double max);
point ray_at(const Ray *r, double t);

bool box_hit(const Hittable *hittable, const Ray *ray, double t_min, double t_max,
             Record *hitrecord);
bool list_hit(const Hittable *hittable, const Ray *ray, double t_min, double t_max,
              Record *hitrecord);

void hr_set_face_normal(Record *hr, const Ray *r, vec3 n);

Camera camera_init(point from, point to, vec3 vup, double vfov, double aspect_ratio,
                   double aperture, double focus_dist);
Ray camera_get_ray(const Camera *c, double s, double t);

#endif // CORE_H

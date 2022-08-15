#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Scene.h"
#include "Material.h"
#include "hittable/Box.h"
#include "hittable/KDTree.h"
#include "hittable/List.h"
#include "hittable/Plane.h"
#include "hittable/Sphere.h"
#include "hittable/Triangle.h"
#include "array.h"
#include "util.h"

#define SKIPBLANK(c)                                                                               \
  while (*c == ' ' || *c == '\t' || *c == '\n')                                                    \
    c++;

typedef enum {
  PARSING_NONE,
  PARSING_MATERIALS,
  PARSING_OBJECTS,
} ParsingStage;

Camera parse_camera_line(char *c);
output parse_output_line(char *c);
Color ray_color(const Ray *r, const Color *bg, Hittable *world, uint max_depth);
spmat *parse_material_line(char *c);
Hittable *parse_object_line(const array_gen *materials, char *c);
void write_color(Color pixel, uint spp);

Scene *_scene_init(FILE *fp) {
  Scene *s = malloc(sizeof(Scene));
  char buf[MAX_BUF_SIZE];
  array_gen *materials = array_gen_init();
  array_gen *objects = array_gen_init();

  ParsingStage stage = PARSING_NONE;

  while (fgets(buf, MAX_BUF_SIZE, fp)) {
    if (!strncmp(buf, "camera:", 7)) {
      s->camera = parse_camera_line(buf);
    } else if (!strncmp(buf, "output:", 7)) {
      s->output = parse_output_line(buf);
    } else if (!strncmp(buf, "materials:", 10)) {
      stage = PARSING_MATERIALS;
    } else if (!strncmp(buf, "objects:", 8)) {
      stage = PARSING_OBJECTS;
    } else if (stage == PARSING_MATERIALS && (buf[0] == ' ' || buf[0] == '\t')) {
      array_gen_push(materials, parse_material_line(buf));
    } else if (stage == PARSING_OBJECTS && (buf[0] == ' ' || buf[0] == '\t')) {
      array_gen_push(objects, parse_object_line(materials, buf));
    }
  }

  s->world = list_init();

  for (size_t i = 0; i < objects->size; i++)
    list_push((List *)s->world, array_gen_get(objects, i));

  array_gen_destroy(materials);
  array_gen_destroy(objects);

  return s;
}

Scene *scene_init() { return _scene_init(stdin); }

Scene *scene_init_file(const char *path) {
  FILE *fp = fopen(path, "r");

  if (fp)
    return _scene_init(fp);

  fclose(fp);

  return NULL;
}

void scene_render(const Scene *s) {
  int i, j, k;
  Color bg_color = {0,0,0};
  printf("P3\n%d %d\n255\n", s->output.width, s->output.height);

  for (j = s->output.height - 1; j >= 0; j--) {
    fprintf(stderr, "\rWriting lines: %d / %d", s->output.height - j, s->output.height);
    fflush(stderr);

    for (i = 0; i < s->output.width; i++) {
      Color pixel = {0, 0, 0};

      for (k = 0; k < s->output.samples_per_pixel; k++) {
        real u = (i + rnd()) / (s->output.width - 1);
        real v = (j + rnd()) / (s->output.height - 1);
        Ray r = camera_get_ray(&s->camera, u, v);
        pixel = vec3_add(pixel, ray_color(&r, &bg_color, s->world, s->output.max_depth));
      }

      write_color(pixel, s->output.samples_per_pixel);
    }
  }

  fprintf(stderr, ".\nDone\n");
}

Color ray_color(const Ray *r, const Color *bg, Hittable *world, uint depth) {
  if (depth <= 0)
    return (Color){0, 0, 0};

  Record rec;

  if (!world->hit(world, r, 0.001, INFINITY, &rec))
    return *bg;

  Material *m = rec.sm ? rec.sm->m : NULL;
  Ray scattered;
  Color attenuation;
  Color emitted = m->emitted(m);

  if (!m->scatter(m, r, &rec, &attenuation, &scattered))
    return emitted;

  return vec3_add(emitted, vec3_prod(attenuation, ray_color(&scattered, bg, world, depth - 1)));
}

Camera parse_camera_line(char *c) {
  char *p = c + 7; // skip 'camera:'

  // camera properties
  Point from = {0, 1, -1}, to = {0, 0, 0};
  Vec3 vup = {0, 1, 0};
  real vfov = 45, aspect_ratio = 16.0 / 9.0, aperture = 0;
  real focus_dist = -1;

  while (*p) {
    while (*p == ' ' || *p == '\n')
      p++;

    if (!strncmp(p, "from=", 5)) {
      p += 5;
      from = parse_vec3(p, &p);
    } else if (!strncmp(p, "to=", 3)) {
      p += 3;
      to = parse_vec3(p, &p);
    } else if (!strncmp(p, "vup=", 4)) {
      p += 4;
      vup = parse_vec3(p, &p);
    } else if (!strncmp(p, "vfov=", 5)) {
      p += 5;
      vfov = strtod(p, &p);
    } else if (!strncmp(p, "ratio=", 6)) {
      p += 6;
      aspect_ratio = strtod(p, &p);
    } else if (!strncmp(p, "aperture=", 9)) {
      p += 9;
      aperture = strtod(p, &p);
    } else if (!strncmp(p, "focus=", 6)) {
      p += 6;
      focus_dist = strtod(p, &p);
    }
  }

  if (focus_dist < 0) {
    focus_dist = sqrt(vec3_norm2(vec3_sub(from, to)));
  }

  return camera_init(from, to, vup, vfov, aspect_ratio, aperture, focus_dist);
}

output parse_output_line(char *c) {
  char *p = c + 7; // skip 'output:'

  // output properties
  output o = {640, 360, 20, 6};

  while (*p) {
    while (*p == ' ' || *p == '\n')
      p++;

    if (!strncmp(p, "width=", 6)) {
      p += 6;
      o.width = strtoul(p, &p, 10);
    } else if (!strncmp(p, "height=", 7)) {
      p += 7;
      o.height = strtoul(p, &p, 10);
    } else if (!strncmp(p, "spp=", 4)) {
      p += 4;
      o.samples_per_pixel = strtoul(p, &p, 10);
    } else if (!strncmp(p, "depth=", 6)) {
      p += 6;
      o.max_depth = strtoul(p, &p, 10);
    }
  }

  return o;
}

spmat *parse_material_line(char *c) {
  spmat *m = NULL;

  while (*c && !m) {
    SKIPBLANK(c);

    if (!strncmp(c, "lambertian=", 11)) {
      c += 11;
      Vec3 albedo = parse_vec3(c, NULL);
      m = lambertian_init(albedo);
    } else if (!strncmp(c, "dielectric:", 11)) {
      Vec3 albedo = {1, 1, 1};
      float ir = 1;

      c += 11;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "albedo=", 7)) {
          c += 7;
          albedo = parse_vec3(c, &c);
        } else if (!strncmp(c, "ir=", 3)) {
          c += 3;
          ir = strtof(c, &c);
        }
      }

      m = dielectric_init(albedo, ir);
    } else if (!strncmp(c, "metal:", 6)) {
      Vec3 albedo = {1, 1, 1};
      float fuzz = 0;

      c += 6;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "albedo=", 7)) {
          c += 7;
          albedo = parse_vec3(c, &c);
        } else if (!strncmp(c, "fuzz=", 3)) {
          c += 3;
          fuzz = strtof(c, &c);
        }
      }

      m = metal_init(albedo, fuzz);
    } else if (!strncmp(c, "light=", 6)) {
      c += 6;
      Vec3 albedo = parse_vec3(c, NULL);
      m = diffuse_light_init(albedo);
    }
  }

  return m;
}

Hittable *parse_object_line(const array_gen *materials, char *c) {
  Hittable *h = NULL;

  while (*c && !h) {
    SKIPBLANK(c);

    if (!strncmp(c, "sphere:", 7)) {
      Vec3 center = {0, 0, 0};
      float radius = 1;
      size_t i = 0;

      c += 7;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "center=", 7)) {
          c += 7;
          center = parse_vec3(c, &c);
        } else if (!strncmp(c, "radius=", 7)) {
          c += 7;
          radius = strtof(c, &c);
        } else if (!strncmp(c, "material=", 9)) {
          c += 9;
          i = strtoul(c, &c, 10);
        }
      }

      h = sphere_init(center, radius, array_gen_get(materials, i));
    } else if (!strncmp(c, "box:", 4)) {
      Vec3 cback = {0, 0, 0}, cfront = {1, 1, 1};
      size_t i = 0;

      c += 4;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "cback=", 6)) {
          c += 6;
          cback = parse_vec3(c, &c);
        } else if (!strncmp(c, "cfront=", 7)) {
          c += 7;
          cfront = parse_vec3(c, &c);
        } else if (!strncmp(c, "material=", 9)) {
          c += 9;
          i = strtoul(c, &c, 10);
        }
      }

      h = box_init(cback, cfront, array_gen_get(materials, i));
    } else if (!strncmp(c, "plane:", 6)) {
      Vec3 origin = {0, 0, 0}, normal = {0, 1, 0};
      size_t i = 0;

      c += 6;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "origin=", 7)) {
          c += 7;
          origin = parse_vec3(c, &c);
        } else if (!strncmp(c, "normal=", 7)) {
          c += 7;
          normal = parse_vec3(c, &c);
        } else if (!strncmp(c, "material=", 9)) {
          c += 9;
          i = strtoul(c, &c, 10);
        }
      }

      h = plane_init(origin, normal, array_gen_get(materials, i));
    } else if (!strncmp(c, "triangle:", 9)) {
      Vec3 triangle_vertex[3] = {{0, 0, 0}, {2, 0, 0}, {1, 1, 0}};
      size_t i = 0, j = 0;

      c += 9;

      while (*c && j < 3) {
        SKIPBLANK(c);
        triangle_vertex[j++] = parse_vec3(c, &c);
      }

      SKIPBLANK(c);

      if (!strncmp(c, "material=", 9)) {
        c += 9;
        i = strtoul(c, &c, 10);
      }

      h = triangle_init(triangle_vertex[0], triangle_vertex[1], triangle_vertex[2],
                        array_gen_get(materials, i));
    } else if (!strncmp(c, "mesh:", 5)) {
      char objpath[MAX_BUF_SIZE], *end;
      size_t leafs = 1000;
      size_t i = 0;

      c += 5;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "objpath=", 8)) {
          c += 8;
          end = strfind(c, ' ');
          strncpy(objpath, c, end - c);
          objpath[end-c] = '\0';
          c = end;
        } else if (!strncmp(c, "leafs=", 6)) {
          c += 6;
          leafs = strtoul(c, &c, 10);
        } else if (!strncmp(c, "material=", 9)) {
          c += 9;
          i = strtoul(c, &c, 10);
        }
      }

      h = kdtree_init_from_file(objpath, leafs, array_gen_get(materials, i));
    }
  }

  return h;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

#include "Scene.h"

#define MAX_BUF_SIZE 256

camera parse_camera_line(char *c);
output parse_output_line(char *c);
color ray_color(const ray *r, Hittable *world, u_int16_t max_depth);
void write_color(color pixel, u_int16_t spp);

Scene *_scene_init(FILE *fp) {
  Scene *s = malloc(sizeof(Scene));
  char buf[MAX_BUF_SIZE];

  while (fgets(buf, MAX_BUF_SIZE, fp)) {
    if (!strncmp(buf, "camera:", 7)) {
      s->camera = parse_camera_line(buf);
    } else if (!strncmp(buf, "output:", 7)) {
      s->output = parse_output_line(buf);
    }
  }

  return s;
}

Scene *scene_init() { return _scene_init(stdin); }

Scene *scene_init_file(const char *path) {
  FILE *fp = fopen(path, "r");

  if (fp)
    return _scene_init(fp);

  return NULL;
}

void scene_render(const Scene *s) {
  int16_t i, j, k;
  printf("P3\n%d %d\n255\n", s->output.width, s->output.height);

  for (j = s->output.height - 1; j >= 0; j--) {
    fprintf(stderr, "\rWriting lines: %d / %d", s->output.height - j,
            s->output.height);
    fflush(stderr);

    for (i = 0; i < s->output.width; i++) {
      color pixel = {0, 0, 0};

      for (k = 0; k < s->output.samples_per_pixel; k++) {
        double u = (i + random_double()) / (s->output.width - 1);
        double v = (j + random_double()) / (s->output.height - 1);
        ray r = camera_get_ray(&s->camera, u, v);
        pixel = vec3_add(pixel, ray_color(&r, s->world, s->output.max_depth));
      }

      write_color(pixel, s->output.samples_per_pixel);
    }
  }
}

color ray_color(const ray *r, Hittable *world, u_int16_t depth) {
  if (depth <= 0)
    return (color){0, 0, 0};

  HitRecord rec;

  if (world->hit(world, r, 0.001, INFINITY, &rec))
    return (color){.3, .4, .5};

  // Gradiente
  vec3 unit_dir = normalized(r->direction);
  double t = .5 * (unit_dir.y + 1);
  color bg_color = vec3_add(vec3_scale(1.0 - t, (color){1, 1, 1}),
                            vec3_scale(t, (color){.5, .7, 1}));

  return bg_color;
}

camera parse_camera_line(char *c) {
  char *p = c + 7; // skip 'camera:'

  // camera properties
  point from = {0, 1, -1}, to = {0, 0, 0};
  vec3 vup = {0, 1, 0};
  double vfov = 45, aspect_ratio = 16.0 / 9.0, aperture = 0;
  double focus_dist = sqrt(vec3_norm2(vec3_sub(from, to)));

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
      o.width = (u_int16_t)strtof(p, &p);
    } else if (!strncmp(p, "height=", 7)) {
      p += 7;
      o.height = (u_int16_t)strtof(p, &p);
    } else if (!strncmp(p, "spp=", 4)) {
      p += 4;
      o.samples_per_pixel = (u_int16_t)strtof(p, &p);
    } else if (!strncmp(p, "depth=", 6)) {
      p += 6;
      o.max_depth = (u_int16_t)strtof(p, &p);
    }
  }

  return o;
}

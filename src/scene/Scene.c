#include "Scene.h"
#include "../structures/array.h"
#include "../surfaces.h"
#include "../vec3.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SKIPBLANK(c)                                                                     \
  while (*c == ' ' || *c == '\t' || *c == '\n')                                          \
    c++;

Scene *scene_new_from_stream(FILE *fp);
Camera parse_camera_line(char *c);
Output parse_output_line(char *c);
Material *parse_material_line(char *c);
Surface *parse_surface_line(char *c, Material **materials);
void write_color(Color pixel, uint spp);

Scene *scene_new_from_stdin() { return scene_new_from_stream(stdin); }

Scene *scene_new_from_file(const char *path) {
  FILE *fp = fopen(path, "r");

  if (fp)
    return scene_new_from_stream(fp);

  fclose(fp);

  return NULL;
}

void scene_dump(Scene *s, FILE *f) {
  int i, j, h = s->output.height, w = s->output.width, spp = s->output.samples_per_pixel;
  Color *image = s->framebuffer;

  fprintf(f, "P3\n%d %d\n255\n", s->output.width, s->output.height);

  for (j = h - 1; j >= 0; j--)
    for (i = 0; i < w; i++)
      write_color(image[j * w + i], spp);
}

void scene_free(Scene *s) {
  list_free((Surface *)s->world);
  free(s->framebuffer);
  free(s);
}

typedef enum {
  PARSING_NONE,
  PARSING_MATERIALS,
  PARSING_OBJECTS,
} ParsingStage;

Scene *scene_new_from_stream(FILE *fp) {
  Scene *s = malloc(sizeof(Scene));
  char buf[MAX_BUF_SIZE];
  Material **materials = NULL;
  Surface **surfaces = NULL;
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
      arr_push(materials, parse_material_line(buf));
    } else if (stage == PARSING_OBJECTS && (buf[0] == ' ' || buf[0] == '\t')) {
      arr_push(surfaces, parse_surface_line(buf, materials));
    }
  }

  s->world = list_new(surfaces);
  s->framebuffer = malloc(sizeof(Color) * s->output.height * s->output.width);

  arr_free(materials);

  return s;
}

Camera parse_camera_line(char *c) {
  char *p = c + 7; // skip 'camera:'

  // camera properties
  Point from = V(0, 1, -1), to = V3(0);
  Vec3 vup = V(0, 1, 0);
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

  return camera_new(from, to, vup, vfov, aspect_ratio, aperture, focus_dist);
}

Output parse_output_line(char *c) {
  char *p = c + 7; // skip 'output:'

  // output properties
  Output o = {640, 360, 20, 6};

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

Material *parse_material_line(char *c) {
  Material *m = NULL;

  while (*c && !m) {
    SKIPBLANK(c);

    if (!strncmp(c, "lambertian=", 11)) {
      c += 11;
      Vec3 albedo = parse_vec3(c, NULL);
      m = lambertian_new(albedo);
    } else if (!strncmp(c, "dielectric:", 11)) {
      Vec3 albedo = V3(1);
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

      m = dielectric_new(albedo, ir);
    } else if (!strncmp(c, "metal:", 6)) {
      Vec3 albedo = V3(1);
      float fuzz = 0;

      c += 6;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "albedo=", 7)) {
          c += 7;
          albedo = parse_vec3(c, &c);
        } else if (!strncmp(c, "fuzz=", 5)) {
          c += 5;
          fuzz = strtof(c, &c);
        }
      }

      m = metal_new(albedo, fuzz);
    } else if (!strncmp(c, "light=", 6)) {
      c += 6;
      Vec3 albedo = parse_vec3(c, NULL);
      m = light_new(albedo);
    }
  }

  return m;
}

Surface *parse_surface_line(char *c, Material **materials) {
  Surface *h = NULL;

  while (*c && !h) {
    SKIPBLANK(c);

    if (!strncmp(c, "sphere:", 7)) {
      Vec3 center = V3(0);
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

      h = (Surface *)sphere_new(center, radius, materials[i]);
    } else if (!strncmp(c, "rect:", 5)) {
      enum plane p = XY;
      real x1_min = 0, x1_max = 1, x2_min = 0, x2_max = 1, x3 = 0;
      bool inv = false;
      size_t i = 0;

      c += 5;

      while (*c) {
        SKIPBLANK(c);

        if (*c == 'X' || *c == 'Y' || *c == 'Z') {
          c++;
          p = *c == 'X' ? ZX : (*c == 'Y' ? XY : YZ);
          c++;
        } else if (!strncmp(c, "x1=", 3)) {
          c += 3;
          x1_min = strtof(c, &c);
          c++;
          x1_max = strtof(c, &c);
        } else if (!strncmp(c, "x2=", 3)) {
          c += 3;
          x2_min = strtof(c, &c);
          c++;
          x2_max = strtof(c, &c);
        } else if (!strncmp(c, "x3=", 3)) {
          c += 3;
          x3 = strtof(c, &c);
        } else if (!strncmp(c, "inverted", 8)) {
          c += 8;
          inv = true;
        } else if (!strncmp(c, "material=", 9)) {
          c += 9;
          i = strtoul(c, &c, 10);
        }
      }

      h = (Surface *)aarect_new(AARP_(p, x1_min, x1_max, x2_min, x2_max, x3, inv),
                                materials[i]);
    } else if (!strncmp(c, "box:", 4)) {
      Vec3 cback = V3(0), cfront = V3(1);
      size_t i = 0;

      c += 4;

      while (*c) {
        SKIPBLANK(c);

        if (!strncmp(c, "pmin=", 5)) {
          c += 5;
          cback = parse_vec3(c, &c);
        } else if (!strncmp(c, "pmax=", 5)) {
          c += 5;
          cfront = parse_vec3(c, &c);
        } else if (!strncmp(c, "material=", 9)) {
          c += 9;
          i = strtoul(c, &c, 10);
        }
      }

      h = (Surface *)aabox_new(cback, cfront, materials[i]);
    } else if (!strncmp(c, "plane:", 6)) {
      Vec3 origin = V3(0), normal = V(0, 1, 0);
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

      h = (Surface *)plane_new(origin, normal, materials[i]);
    } else if (!strncmp(c, "triangle:", 9)) {
      Vec3 triangle_vertex[3] = {0};
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

      h = (Surface *)triangle_new(triangle_vertex[0], triangle_vertex[1],
                                  triangle_vertex[2], materials[i]);
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
          objpath[end - c] = '\0';
          c = end;
        } else if (!strncmp(c, "leafs=", 6)) {
          c += 6;
          leafs = strtoul(c, &c, 10);
        } else if (!strncmp(c, "material=", 9)) {
          c += 9;
          i = strtoul(c, &c, 10);
        }
      }

      h = (Surface *)triangle_mesh_new_from_file(objpath, leafs, materials[i]);
    }
  }

  return h;
}

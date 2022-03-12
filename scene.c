#include <stdio.h>
#include <string.h>
#include <util.h>

#include "scene.h"

#define MAX_BUF_SIZE 256

camera parse_camera_line(char *c);
output parse_output_line(char *c);

scene scene_init() {
  scene s;
  FILE *fp = stdin;
  char buf[MAX_BUF_SIZE];

  while (fgets(buf, MAX_BUF_SIZE, fp)) {
    printf("printing buffer: %s", buf);
    if (!strncmp(buf, "camera:", 7)) {
      s.camera = parse_camera_line(buf);
    } else if (!strncmp(buf, "output:", 7)) {
      s.output = parse_output_line(buf);
    }
  }

  return s;
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

  // camera properties
  output o = { 640, 360, 20, 6 };

  while (*p) {
    while (*p == ' ' || *p == '\n')
      p++;

    if (!strncmp(p, "width=", 6)) {
      p += 6;
      o.width = (u_int16_t) strtof(p, &p);
    } else if (!strncmp(p, "height=", 7)) {
      p += 7;
      o.height = (u_int16_t) strtof(p, &p);
    } else if (!strncmp(p, "spp=", 4)) {
      p += 4;
      o.samples_per_pixel = (u_int16_t) strtof(p, &p);
    } else if (!strncmp(p, "depth=", 6)) {
      p += 5;
      o.max_depth = (u_int16_t) strtof(p, &p);
    }
  }

  return o;
}

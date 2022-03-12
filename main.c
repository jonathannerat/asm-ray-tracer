#include <stdio.h>
#include <sys/types.h>

#include "scene.h"
#include "util.h"

int main(int argc, char **argv) {
  scene s = scene_init();

  vec3_debug(s.camera.u);

  return 0;
}

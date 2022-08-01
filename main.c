#ifndef DEBUG_ASM
#include "Scene.h"
#endif

#include <math.h>
#include "util.h"

int main(int argc, char **argv) {
  Camera c = camera_init(V(0,0,0), V(1,1,1), V(0,1,0), 45, 16.0/9.0, 1.0, 1.0);
  return 0;
}

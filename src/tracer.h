#ifndef TRACER_H
#define TRACER_H

#include "scene/Scene.h"

void tracer_c(Scene *, bool);
void tracer_asm(Camera, Output, List *, Color *, bool);
void report(int, int);

void mtsrand(int);
float mtfrand();
Vec3 mtfrand_vec();

#endif /* end of include guard: TRACER_H */

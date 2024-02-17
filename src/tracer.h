#ifndef TRACER_H
#define TRACER_H

#include "scene/Scene.h"

void tracer_c(Scene *);
void tracer_asm(Camera, Output, List *, Color *);

void mtsrand(int);
float mtfrand();

#endif /* end of include guard: TRACER_H */

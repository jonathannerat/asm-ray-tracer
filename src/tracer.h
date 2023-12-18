#ifndef TRACER_H
#define TRACER_H

#include "scene/Scene.h"

void tracer_c(Scene *);

void tracer_asm(Camera, Output, List *, Color *);

#endif /* end of include guard: TRACER_H */

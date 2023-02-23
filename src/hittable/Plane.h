#ifndef HITTABLE_PLANE_H
#define HITTABLE_PLANE_H

#include "../core.h"

Hittable *plane_init(Point origin, Vec3 normal, spmat *sm);

#endif // HITTABLE_PLANE_H

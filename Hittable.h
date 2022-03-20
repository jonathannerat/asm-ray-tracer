#ifndef HITTABLE_H
#define HITTABLE_H

#include "hittable/Record.h"

typedef struct _hittable Hittable;
typedef bool_ (*hit_method)(const Hittable *o, const ray *r, double t_min,
                            double t_max, Record *hr);

struct _hittable {
  hit_method hit;
};

#endif // HITTABLE_H

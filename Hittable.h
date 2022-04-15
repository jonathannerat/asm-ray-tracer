#ifndef HITTABLE_H
#define HITTABLE_H

#include "hittable/Record.h"

#define DESTROY(e) ((Hittable *)(e))->destroy((Hittable *)e)

typedef struct _hittable Hittable;
typedef bool (*hit_method)(const Hittable *o, const ray *r, double t_min,
                            double t_max, Record *hr);
typedef void (*destroy_method)(Hittable *);

struct _hittable {
  hit_method hit;
  destroy_method destroy;
};

#endif // HITTABLE_H

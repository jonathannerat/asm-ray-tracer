#ifndef HITTABLE_H
#define HITTABLE_H

#include "hittable/Record.h"

#define DESTROY(e) ((Hittable *)(e))->destroy((Hittable *)e)

typedef struct _hittable Hittable;

struct _box;

struct _hittable {
  bool (*hit)(const Hittable *o, const ray *r, double t_min, double t_max, Record *hr);

  void (*destroy)(Hittable *);

  struct _box *(*bbox)(const Hittable *);
};

#endif // HITTABLE_H

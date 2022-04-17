#ifndef HITTABLE_H
#define HITTABLE_H

#include "hittable/Record.h"

#define DESTROY(e) ((Hittable *)(e))->destroy((Hittable *)e)
#define HIT(e, ...) ((Hittable *)(e))->hit((Hittable *)(e), __VA_ARGS__)

typedef struct _hittable Hittable;

struct _box;

struct _hittable {
  bool (*hit)(const Hittable *o, const ray *r, double t_min, double t_max, Record *hr);

  void (*destroy)(Hittable *);

  struct _box *(*bbox)(const Hittable *);

  point refp;
};

#endif // HITTABLE_H

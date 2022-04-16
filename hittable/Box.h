#ifndef HITTABLE_BOX_H
#define HITTABLE_BOX_H

#include "List.h"
#include "Plane.h"

struct _box {
  Hittable _hittable;
  point cback;
  point cfront;
  List *faces;
  shrmat sm;
};

typedef struct _box Box;

/** Initialize a new axis aligned box delimited by two (*distinct) points
 *
 * * In this context, we define two points as distinct if they are different at
 * every coordinate. This is required because this box is axis aligned, so if
 * two coordinates are the same, the box would have width zero at that axis.
 *
 * @param p1 first point
 * @param p2 second point
 * @param m box material (owned)
 * @return the Box instance
 */
Hittable *box_init(point p1, point p2, shrmat sm);

/** Free the box instance */
void box_destroy(Hittable *box);

Box *box_join(const Box *a, const Box *b);

/** Checks if the point p is inside box
 *
 * @param box box boundary
 * @param p point to check
 * @return true iff it's inside
 */
inline bool box_is_inside(const Box *self, point p) {
  return self->cback.x - EPS <= p.x && p.x <= self->cfront.x + EPS &&
         self->cback.y - EPS <= p.y && p.y <= self->cfront.y + EPS &&
         self->cback.z - EPS <= p.z && p.z <= self->cfront.z + EPS;
}

#endif // HITTABLE_BOX_H

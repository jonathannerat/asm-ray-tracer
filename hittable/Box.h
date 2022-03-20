#ifndef HITTABLE_BOX_H
#define HITTABLE_BOX_H

#include "../Hittable.h"
#include "List.h"
#include "Plane.h"

typedef struct {
  Hittable _hittable;
  point cback;
  point cfront;
  List *faces;
} Box;

/** Initialize a new axis aligned box delimited by two (*distinct) points
 *
 * * In this context, we define two points as distinct if they are different at
 * every coordinate. This is required because this box is axis aligned, so if
 * two coordinates are the same, the box would have width zero at that axis.
 *
 * @param p1 first point
 * @param p2 second point
 * @return the Box instance
 */
Box *box_init(point p1, point p2);

/** Free the box instance */
void box_destroy(Box *box);

#endif // HITTABLE_BOX_H

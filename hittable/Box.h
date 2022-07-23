#ifndef HITTABLE_BOX_H
#define HITTABLE_BOX_H

#include "core.h"
#include "util.h"

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
Hittable *box_init(Point p1, Point p2, spmat *sm);

Box *box_join(const Box *a, const Box *b);

bool box_is_inside(const Box *self, Point p);

#endif // HITTABLE_BOX_H

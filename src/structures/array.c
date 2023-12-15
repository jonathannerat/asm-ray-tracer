#include <stdlib.h>

#include "array.h"

void *arr_growf(void *arr, uint elem_size, uint growth_size) {
  void *bigger_arr;
  uint new_size;

  // we have to alocate at least this many elements
  uint min_cap = arr_len(arr) + growth_size;
  // we currently can alocate this many elements
  uint capacity = arr_cap(arr);

  if (min_cap < 2 * capacity) {
    // doubling our current capacity is enough
    min_cap = 2 * capacity;
  } else if (min_cap < ARRAY_INITIAL_CAPACITY) {
    // if its not enough, maybe it's because we're empty
    min_cap = ARRAY_INITIAL_CAPACITY;
  } // otherwise, we might be needing a lot of space, then we just use min_cap

  // we make enough space for the header
  new_size = elem_size * min_cap + sizeof(struct array_header);
  bigger_arr = realloc(arr ? arr_header(arr) : 0, new_size);
  // we offset the pointer, so we can directly access elements
  bigger_arr = (char *)bigger_arr + sizeof(struct array_header);

  if (!arr) {
    // arr was NULL, then bigger_arr won't have length set
    arr_header(bigger_arr)->length = 0;
  }

  // regardless we need to set to new capacity
  arr_header(bigger_arr)->capacity = min_cap;

  return bigger_arr;
}

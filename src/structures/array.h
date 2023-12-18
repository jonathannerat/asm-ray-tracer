#ifndef ARRAY_H
#define ARRAY_H

#define ARRAY_INITIAL_CAPACITY 16

// inspired by https://github.com/nothings/stb/blob/master/stb_ds.h

struct array_header {
  unsigned int length;
  unsigned int capacity;
};

#define arr_header(a) ((struct array_header *)(a)-1)
#define arr_len(a) ((a) ? arr_header(a)->length : 0)
#define arr_cap(a) ((a) ? arr_header(a)->capacity : 0)
#define arr_push(a, v) (arr_maybegrow(a, 1), (a)[arr_header(a)->length++] = (v))
#define arr_maybegrow(a, n)                                                              \
  (!(a) || arr_len(a) + (n) > arr_cap(a) ? (arr_grow(a, n), 1) : 0)
#define arr_grow(a, n) ((a) = arr_growf(a, sizeof *(a), (n)))
#define arr_free(a) ((a) ? (free(arr_header(a)), 0) : 0, (a) = NULL)
void *arr_growf(void *arr, unsigned int elem_size, unsigned int growth_size);

#endif // ARRAY_H

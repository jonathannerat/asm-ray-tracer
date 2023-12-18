#ifndef SHPT_H
#define SHPT_H

#include "../vec3.h"

struct shpt_header {
  uint ref_count;
  uint _; // padding
};

#define shpt_header(p) ((struct shpt_header *)(p)-1)
#define shpt_new(Type)                                                                   \
  ({                                                                                     \
    Type *tmp =                                                                          \
      (Type *)((struct shpt_header *)malloc(sizeof(Type) + sizeof(struct shpt_header)) + \
               1);                                                                       \
    shpt_header(tmp)->ref_count = 0;                                                     \
    tmp;                                                                                 \
  })
#define shpt_get(p) ((p) ? (shpt_header(p)->ref_count++, (p)) : (p))
#define shpt_free(p)                                                                     \
  ((p)                                                                                   \
     ? (--(shpt_header(p)->ref_count) == 0 ? (free(shpt_header(p)), (p) = NULL, 0) : 0)  \
     : 0)

#endif /* end of include guard: SHPT_H */

#ifndef ARRAY_H
#define ARRAY_H

#include "core.h"

#define ARRAY_INITIAL_CAPACITY 16

typedef struct {
  Vec3 *data;
  uint size;
  uint cap;
} array_vec3;

typedef struct {
  void **data;
  uint size;
  uint cap;
} array_gen;

array_vec3 *array_vec3_init();
bool array_vec3_push(array_vec3 *arr, Vec3 v);
Vec3 array_vec3_get(const array_vec3 *arr, uint i);
void array_vec3_destroy(array_vec3 *arr);

array_gen *array_gen_init();
bool array_gen_push(array_gen *arr, void *v);
void *array_gen_get(const array_gen *arr, uint i);
void array_gen_destroy(array_gen *arr);

#endif // ARRAY_H

#ifndef ARRAY_H
#define ARRAY_H

#include "vec3.h"

#define ARRAY_INITIAL_CAPACITY 16

typedef struct {
  vec3 *data;
  size_t size;
  size_t cap;
} array_vec3;

typedef struct {
  void **data;
  size_t size;
  size_t cap;
} array_gen;

array_vec3 *array_vec3_init();
bool array_vec3_push(array_vec3 *arr, vec3 v);
vec3 array_vec3_get(const array_vec3 *arr, size_t i);
void array_vec3_destroy(array_vec3 *arr);

array_gen *array_gen_init();
bool array_gen_push(array_gen *arr, void *v);
void *array_gen_get(const array_gen *arr, size_t i);
void array_gen_destroy(array_gen *arr);

#endif // ARRAY_H

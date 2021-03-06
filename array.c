#include <stdlib.h>

#include "array.h"
#include "util.h"

array_vec3 *array_vec3_init() {
  array_vec3 *arr = malloc(sizeof(array_vec3));

  arr->size = 0;
  arr->cap = ARRAY_INITIAL_CAPACITY;
  arr->data = malloc(sizeof(Vec3) * arr->cap);

  return arr;
}

bool array_vec3_push(array_vec3 *arr, Vec3 v) {
  if (arr->size == arr->cap) {
    Vec3 *data = realloc(arr->data, sizeof(Vec3) * arr->cap * 2);
    if (!data)
      return false;

    if (data != arr->data)
      arr->data = data;
    arr->cap *= 2;
  }

  arr->data[arr->size++] = v;

  return true;
}

Vec3 array_vec3_get(const array_vec3 *arr, uint i) {
  if (i >= arr->size)
    return V(NAN, NAN, NAN);

  return arr->data[i];
}

void array_vec3_destroy(array_vec3 *arr) {
  free(arr->data);
  free(arr);
}

// For pointers

array_gen *array_gen_init() {
  array_gen *arr = malloc(sizeof(array_gen));

  arr->size = 0;
  arr->cap = ARRAY_INITIAL_CAPACITY;
  arr->data = malloc(sizeof(void *) * arr->cap);

  return arr;
}

bool array_gen_push(array_gen *arr, void *v) {
  if (arr->size == arr->cap) {
    void **data = realloc(arr->data, sizeof(void *) * arr->cap * 2);
    if (!data)
      return false;

    if (data != arr->data)
      arr->data = data;
    arr->cap *= 2;
  }

  arr->data[arr->size++] = v;

  return true;
}

void *array_gen_get(const array_gen *arr, uint i) {
  if (i >= arr->size)
    return NULL;

  return arr->data[i];
}

void array_gen_destroy(array_gen *arr) {
  free(arr->data);
  free(arr);
}

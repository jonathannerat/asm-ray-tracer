#include "array.h"

array_vec3 *array_vec3_init() {
  array_vec3 *arr = malloc(sizeof(array_vec3));

  arr->size = 0;
  arr->cap = ARRAY_INITIAL_CAPACITY;
  arr->data = malloc(sizeof(vec3) * arr->cap);

  return arr;
}

bool array_vec3_push(array_vec3 *arr, vec3 v) {
  if (arr->size == arr->cap) {
    vec3 *data = realloc(arr->data, sizeof(vec3) * arr->cap * 2);
    if (!data)
      return false;

    if (data != arr->data)
      arr->data = data;
    arr->cap *= 2;
  }

  arr->data[arr->size++] = v;

  return true;
}

vec3 array_vec3_get(const array_vec3 *arr, size_t i) {
  if (i >= arr->size)
    return (vec3){NAN, NAN, NAN};

  return arr->data[i];
}

void array_vec3_destroy(array_vec3 *arr) { free(arr->data); }

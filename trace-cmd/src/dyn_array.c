#include <stdlib.h>
#include "dyn_array.h"

#define INITIAL_CAPACITY 100

struct _DynArray {
  void** array;
  int capacity;
  int size;
};


DynArray* DynArray_new() {
  DynArray* result = (DynArray*) malloc(sizeof(DynArray));
  result->capacity = INITIAL_CAPACITY;
  result->array = (void**) malloc(sizeof(void*) * result->capacity);
  result->size = 0;

  return result;
}

void DynArray_free(DynArray* da) {
  free(da->array);
  free(da);
}

void DynArray_resize(DynArray* da, size_t memsize) {
  da->capacity = memsize;
  da->array = (void**) realloc(da->array, sizeof(void*) * da->capacity);
}

void DynArray_insert(DynArray* da, void* obj) {
  if(da->capacity == da->size) {
    DynArray_resize(da, da->capacity * 2);
  }

  da->array[da->size] = obj;
  da->size++;
}


void DynArray_remove(DynArray* da, int i) {
  if(da->capacity > da->size * 2){
    DynArray_resize(da, da->capacity / 2);
  }

  for(int j = i; j<da->size-1; j++) {
    da->array[j] = da->array[j+1];
  }

  da->size--;
}

void* DynArray_get(DynArray* da, int i) {
  return da->array[i];
}

int DynArray_size(DynArray* da) {
  return da->size;
}

void** DynArray_get_raw_array(DynArray* da){
  return da->array;
}


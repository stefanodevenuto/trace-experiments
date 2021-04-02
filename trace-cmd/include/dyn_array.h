#pragma once

typedef struct _DynArray DynArray;


/**
 *  Create a new DynArray.
 */
DynArray* DynArray_new();

/**
 * Frees the memory alloced by DynArray_new.
 * Note: it does not free the memory of the objects
 *       in the array.
 */ 
void DynArray_free(DynArray* da);

/**
 * Inserts a new object in the dynamic array.
 */
void DynArray_insert(DynArray* da, void* obj);

/**
 * Removes the object in position i
 */
void DynArray_remove(DynArray* da, int i);

/**
 *  Returns the object in position i
 */
void* DynArray_get(DynArray* da, int i);

/** Returns the number of elements currently present
 * in the array.
 */
int DynArray_size(DynArray* da);

/**
 * Return an array of objects
 */
void** DynArray_get_raw_array(DynArray* da);
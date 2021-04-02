#pragma once

typedef struct _SortedArray SortedArray;


/**
 *  Create a new SortedArray.
 *  Parameter: pointer to a function that compares
 *    two objects in the array. The library assumes
 *    that the function returns a value less than 0 if the first 
 *    object is smaller than the first, returns 0
 *    if they are equal, and it returns a value larger than 0 
 *    if the first object is larger than the second.
 */
SortedArray* SortedArray_new();

/**
 * Frees the memory alloced by SortedArray_new.
 * Note: it does not free the memory of the objects
 *       in the array.
 */ 
void SortedArray_free(SortedArray* sa);

/**
 * Inserts a new object in the sorted array.
 */
void SortedArray_insert(SortedArray* sa, unsigned long long* obj);

/**
 * Removes the object in position i
 */
void SortedArray_remove(SortedArray* sa, int i);

/**
 *  Returns the object in position i
 */
void* SortedArray_get(SortedArray* sa, int i);

/** Returns the number of elements currently present
 * in the array.
 */
int SortedArray_size(SortedArray* sa);

unsigned long long** SortedArray_get_raw_array(SortedArray* sa);
int SortedArray_contains(SortedArray* sa, unsigned long long x);

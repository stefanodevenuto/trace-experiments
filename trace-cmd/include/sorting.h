#pragma once

typedef int (*SortingCmp)(void*, void*) ;


/**
 * Sort an array using the Insertion sort alghoritm
 * Parameters: 
 *    1. pointer to a generic array structure
 *    2. the number of elements in the array
 *    3. pointer to a function that compares
 *       two objects in the array. The library assumes
 *       that the function returns a value less than 0 if the first 
 *       object is smaller than the first, returns 0
 *       if they are equal, and it returns a value larger than 0 
 *       if the first object is larger than the second.
 */ 
void insertion_sort(void** array, int n_elem, SortingCmp);


/**
 * Sort an array using the Quick-sort alghoritm
 * Parameters: 
 *    1. pointer to a generic array structure
 *    2. the number of elements in the array
 *    3. pointer to a function that compares
 *       two objects in the array. The library assumes
 *       that the function returns a value less than 0 if the first 
 *       object is smaller than the first, returns 0
 *       if they are equal, and it returns a value larger than 0 
 *       if the first object is larger than the second.
 */ 
void quick_sort(void** array, int n_elem, SortingCmp);

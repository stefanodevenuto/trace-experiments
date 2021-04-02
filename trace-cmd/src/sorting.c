#include <stdio.h>
#include "sorting.h"

#define SWAP(array, i, j) void* tmp = array[i]; \
                          array[i] = array[j]; \
                          array[j] = tmp;

static void partition(void** array, int left, int right, int* less, int* greater, SortingCmp compare){

  int i = left;
  void** pivot = array[left];

  *less = left;
  *greater = right;

  while(i <= *greater){
    if(compare(array[i], pivot) < 0){
      SWAP(array, *less, i)
      *less += 1;
      i++;
    }else if(compare(array[i], pivot) > 0){
      SWAP(array, i, *greater)
      *greater -= 1;
    }else{
      i++;
    }
  }
}

void insertion_sort(void** array, int n_elem, SortingCmp compare){  
  
  if(array == NULL) return;

  for(int i = 1; i < n_elem; i++){
    for(int j = i; j > 0 && compare(array[j - 1],array[j]) > 0; j--){
      SWAP(array, j, j - 1)
    }
  }
}

static void quick_sort_implementation(void** array, int start, int end, SortingCmp compare){
  if(start < end){
    int left;
    int right;
    partition(array, start, end, &left, &right, compare);
    quick_sort_implementation(array, start, left - 1, compare);
    quick_sort_implementation(array, right + 1, end, compare);
  }
}

void quick_sort(void** array, int n_elem, SortingCmp compare){

  if(array == NULL) return;

  quick_sort_implementation(array, 0, n_elem - 1, compare);
}

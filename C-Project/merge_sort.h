#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <stdint.h>
#include <stddef.h>

/* 
   Allocate/free a single global scratch buffer of size n.
   Must be called before/after any calls to parallel_merge_sort_omp().
*/
void parallel_merge_sort_init(size_t n);
void parallel_merge_sort_fini(void);

/* 
   Sorts a[left..right) in place by recursive, task‑based merge sort.
   Must be called inside a single+parallel region:
     #pragma omp parallel
     #pragma omp single nowait
       parallel_merge_sort_omp(a, 0, n);
*/
void parallel_merge_sort_omp(uint32_t *a, size_t left, size_t right);

#endif // MERGE_SORT_H

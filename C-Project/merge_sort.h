 
#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <stdint.h>
#include <stddef.h>

/**
 * Prepare internal buffers for sorting an array of size n.
 * Must be called once before parallel_merge_sort().
 */
void parallel_merge_sort_init(size_t n);

/**
 * Sort a[0..n) in place.  Must be invoked inside:
 *   #pragma omp parallel
 *   #pragma omp single nowait
 *     parallel_merge_sort(a, n);
 */
void parallel_merge_sort(uint32_t *a, size_t n);

/**
 * Release internal buffers.  Call when you’re done.
 */
void parallel_merge_sort_fini(void);

#endif // MERGE_SORT_H
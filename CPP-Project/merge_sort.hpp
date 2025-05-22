// merge_sort.h
#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <cstddef>
#include <cstdint>

// Prepare internal buffers for sorting an array of size n
void parallel_merge_sort_init(size_t n);

// Sort a[0..n) in place
void parallel_merge_sort(uint32_t *a, size_t n);

// Release internal buffers
void parallel_merge_sort_fini(void);

#endif // MERGE_SORT_H
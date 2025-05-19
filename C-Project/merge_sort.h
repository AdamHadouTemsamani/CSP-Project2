#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <stdint.h>
#include <stddef.h>

// Cutoff for spawning tasks; below this, sort inline
#define MERGE_SORT_OMP_THRESHOLD 1024

// Sequential merge of a[left..mid-1] and a[mid..right-1]
void merge(uint32_t *a, size_t left, size_t mid, size_t right);

// OpenMP-driven parallel merge sort
void parallel_merge_sort_omp(uint32_t *a, size_t n);

#endif // MERGE_SORT_H

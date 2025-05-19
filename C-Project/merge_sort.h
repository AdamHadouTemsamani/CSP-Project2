#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <stdint.h>
#include <stddef.h>

void merge(uint32_t *a, size_t left, size_t mid, size_t right);
void parallel_merge_sort_omp(uint32_t *a, size_t left, size_t right);

#endif // MERGE_SORT_H

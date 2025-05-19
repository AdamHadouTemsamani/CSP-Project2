#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <uv.h>

void merge(uint32_t arr[], size_t left, size_t mid, size_t right);
void merge_sort(uint32_t arr[], size_t left, size_t right);

// libuv-based parallel merge-sort:
void merge_sort_uv(uint32_t *arr,
                   size_t left,
                   size_t right,
                   uv_loop_t *loop,
                   uv_mutex_t *mutex);

#endif // MERGE_SORT_H

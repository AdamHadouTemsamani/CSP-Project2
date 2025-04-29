#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <uv.h>

// Original merge-sort:
void merge(int arr[], int left, int mid, int right);
void merge_sort(int arr[], int left, int right);

// libuv-based parallel merge-sort:
void merge_sort_uv(int *arr,
                   int left,
                   int right,
                   uv_loop_t *loop,
                   uv_mutex_t *mutex,
                   int *counter);

#endif // MERGE_SORT_H

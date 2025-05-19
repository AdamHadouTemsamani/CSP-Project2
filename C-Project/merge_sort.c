#include "merge_sort.h"
#include <stdlib.h>
#include <string.h>

/**
 * Standard merge of two sorted halves.
 */
void merge(uint32_t arr[], size_t left, size_t mid, size_t right) {
    size_t left_size = mid - left + 1;
    size_t right_size = right - mid;
    uint32_t *left_arr = malloc(left_size * sizeof(uint32_t));
    uint32_t *right_arr = malloc(right_size * sizeof(uint32_t));
    memcpy(left_arr, arr + left, left_size * sizeof(uint32_t));
    memcpy(right_arr, arr + mid + 1, right_size * sizeof(uint32_t));
    size_t i = 0;
    size_t j = 0;
    size_t k = left;
    while (i < left_size && j < right_size) {
        if (left_arr[i] <= right_arr[j]) {
            arr[k++] = left_arr[i++];
        } else {
            arr[k++] = right_arr[j++];
        }
    }
    if (i < left_size) {
        memcpy(arr + k, left_arr + i, (left_size - i) * sizeof(uint32_t));
    }
    k += left_size - i;
    if (j < right_size) {
        memcpy(arr + k, right_arr + j, (right_size - j) * sizeof(uint32_t));
    }
    free(left_arr);
    free(right_arr);
}

/**
 * Simple recursive merge-sort.
 */
void merge_sort(uint32_t arr[], size_t left, size_t right) {
    if (left >= right) {
        return;
    }
    size_t mid = left + (right - left) / 2;
    merge_sort(arr, left, mid);
    merge_sort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

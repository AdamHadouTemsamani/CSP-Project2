#include "merge_sort.h"
#include <stdlib.h>
#include <string.h>
<<<<<<< HEAD
#include <omp.h>

static uint32_t *scratch = NULL;

void parallel_merge_sort_init(size_t n) {
    scratch = malloc(n * sizeof *scratch);
    if (!scratch) {
        perror("malloc scratch");
        exit(EXIT_FAILURE);
    }
}

void parallel_merge_sort_fini(void) {
    free(scratch);
    scratch = NULL;
}

static void merge(uint32_t *a, size_t left, size_t mid, size_t right) {
    size_t len = right - left;
    /* copy the whole segment into scratch once */
    memcpy(scratch + left, a + left, len * sizeof *a);

    size_t i = left, j = mid, k = left;
    while (i < mid && j < right) {
        a[k++] = (scratch[i] <= scratch[j]) ? scratch[i++] : scratch[j++];
    }
    /* one of these will run, the other zero‑times */
    while (i < mid)  a[k++] = scratch[i++];
    while (j < right) a[k++] = scratch[j++];
}

void parallel_merge_sort_omp(uint32_t *a, size_t left, size_t right) {
    if (right - left <= 1) return;

    size_t mid = left + (right - left) / 2;

    #pragma omp task shared(a)
    parallel_merge_sort_omp(a, left, mid);

    #pragma omp task shared(a)
    parallel_merge_sort_omp(a, mid, right);

    #pragma omp taskwait
    merge(a, left, mid, right);
=======

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
>>>>>>> origin/main
}

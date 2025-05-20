#include "merge_sort.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>

/* When subarray length >= TASK_THRESH, we spawn true tasks;
   below that we recurse serially to avoid overhead. */
#define TASK_THRESH 20000

/* One big scratch buffer of size n, allocated once. */
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

/* Merge a[left..mid) with a[mid..right) into scratch and copy back */
static void merge_range(uint32_t *a, size_t left, size_t mid, size_t right) {
    size_t i = left, j = mid, k = left;
    while (i < mid && j < right) {
        scratch[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    }
    while (i < mid)  scratch[k++] = a[i++];
    while (j < right) scratch[k++] = a[j++];
    memcpy(a + left, scratch + left, (right - left) * sizeof *a);
}

/* Internal recursive sorter on a[left..right) */
static void do_sort(uint32_t *a, size_t left, size_t right) {
    size_t len = right - left;
    if (len <= 1) return;

    size_t mid = left + len/2;

    #pragma omp task shared(a)
    do_sort(a, left,  mid);

    #pragma omp task shared(a)
    do_sort(a, mid, right);

    #pragma omp taskwait

    merge_range(a, left, mid, right);
}

void parallel_merge_sort(uint32_t *a, size_t n) {
    do_sort(a, 0, n);
}
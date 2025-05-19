#include "merge_sort.h"
#include <stdlib.h>
#include <string.h>
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
}

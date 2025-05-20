#include "merge_sort.h"
#include <stdlib.h>
#include <string.h>
#include <omp.h>

void merge(uint32_t *a, size_t left, size_t mid, size_t right) {
    size_t n1 = mid - left;
    size_t n2 = right - mid;
    uint32_t *L = malloc(n1 * sizeof *L);
    uint32_t *R = malloc(n2 * sizeof *R);

    memcpy(L, &a[left], n1 * sizeof *L);
    memcpy(R, &a[mid],  n2 * sizeof *R);

    size_t i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        a[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
    free(L);
    free(R);
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

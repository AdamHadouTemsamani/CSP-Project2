#include "merge_sort.hpp"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <tbb/task_group.h>

static void merge_range(uint32_t *a, size_t left, size_t mid, size_t right) {
size_t n1 = mid - left;
    size_t n2 = right - mid;

    uint32_t* L = (uint32_t*) std::malloc(n1 * sizeof(uint32_t));
    uint32_t* R = (uint32_t*) std::malloc(n2 * sizeof(uint32_t));
    if (!L || !R) {
        std::fprintf(stderr, "malloc failed in merge_range\n");
        std::exit(EXIT_FAILURE);
    }

    std::memcpy(L, a + left, n1 * sizeof(uint32_t));
    std::memcpy(R, a + mid,  n2 * sizeof(uint32_t));

    size_t i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        a[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];

    std::free(L);
    std::free(R);
}

static void do_sort(uint32_t *a, size_t left, size_t right) {
    if (right - left <= 1) return;

    size_t mid = left + (right - left) / 2;

    tbb::task_group tg;
    tg.run([&] { do_sort(a, left, mid); });
    tg.run([&] { do_sort(a, mid, right); });
    tg.wait();

    merge_range(a, left, mid, right);
}

void parallel_merge_sort(uint32_t *a, size_t n) {
    do_sort(a, 0, n);
}
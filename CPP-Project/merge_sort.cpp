#include "merge_sort.hpp"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <tbb/task_group.h>

#define TASK_THRESH 20000

static uint32_t *scratch = nullptr;

void parallel_merge_sort_init(size_t n) {
    scratch = (uint32_t*)std::malloc(n * sizeof *scratch);
    if (!scratch) {
        perror("malloc scratch");
        std::exit(EXIT_FAILURE);
    }
}

void parallel_merge_sort_fini(void) {
    std::free(scratch);
    scratch = nullptr;
}

static void merge_range(uint32_t *a, size_t left, size_t mid, size_t right) {
    size_t i = left, j = mid, k = left;
    while (i < mid && j < right) {
        scratch[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    }
    while (i < mid)  scratch[k++] = a[i++];
    while (j < right) scratch[k++] = a[j++];
    std::memcpy(a + left, scratch + left, (right - left) * sizeof *a);
}

static void do_sort(uint32_t *a, size_t left, size_t right) {
    size_t len = right - left;
    if (len <= 1) return;

    size_t mid = left + len/2;

    if (len >= TASK_THRESH) {
        tbb::task_group tg;
        tg.run([&]{ do_sort(a, left,  mid); });
        tg.run([&]{ do_sort(a, mid,   right); });
        tg.wait();
    } else {
        // serial recursion below threshold
        do_sort(a, left,  mid);
        do_sort(a, mid,   right);
    }

    merge_range(a, left, mid, right);
}

void parallel_merge_sort(uint32_t *a, size_t n) {
    do_sort(a, 0, n);
}
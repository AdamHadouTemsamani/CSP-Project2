#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>
#include "merge_sort.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <maxThreads> <arraySize>\n", argv[0]);
        return 1;
    }
    int max_threads = atoi(argv[1]);
    size_t n = (size_t)atoll(argv[2]);
    if (max_threads < 1 || n < 1) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    // Read binary input
    uint32_t *a = malloc(n * sizeof *a);
    FILE *f = fopen("../random_integers.bin", "rb");
    if (!f || fread(a, sizeof *a, n, f) != n) {
        perror("reading input");
        return 1;
    }
    fclose(f);

    omp_set_num_threads(max_threads);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    #pragma omp parallel
    #pragma omp single nowait
    parallel_merge_sort_omp(a, n);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (n / secs) / 1e6;
    printf("%d,%zu,%.6f,%.3f\n", max_threads, n, secs, mips);

    free(a);
    return 0;
}

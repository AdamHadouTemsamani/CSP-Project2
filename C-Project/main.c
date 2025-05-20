#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>
#include "merge_sort.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <maxThreads> <arraySize>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int max_threads = atoi(argv[1]);
    size_t n        = (size_t)atoll(argv[2]);
    if (max_threads < 1 || n < 1) {
        fprintf(stderr, "Invalid arguments\n");
        return EXIT_FAILURE;
    }

    /* Read binary file of little‑endian uint32_t */
    const char *path = "random_integers.bin";
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }
    uint32_t *a = malloc(n * sizeof *a);
    if (!a) {
        perror("malloc array");
        fclose(f);
        return EXIT_FAILURE;
    }
    if (fread(a, sizeof *a, n, f) != n) {
        fprintf(stderr, "Expected %zu ints but got fewer\n", n);
        fclose(f);
        free(a);
        return EXIT_FAILURE;
    }
    fclose(f);

    omp_set_num_threads(max_threads);
    parallel_merge_sort_init(n);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    #pragma omp parallel
    #pragma omp single nowait
    parallel_merge_sort(a, n);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec)
                + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (n / secs) / 1e6;
    printf("%d,%zu,%.6f,%.3f\n",
           max_threads, n, secs, mips);

    parallel_merge_sort_fini();
    free(a);
    return EXIT_SUCCESS;
}

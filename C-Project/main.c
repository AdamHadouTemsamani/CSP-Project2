#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>
#include "merge_sort.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <maxThreads> <arraySize>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t max_threads = atoi(argv[1]);
    size_t array_size = atoi(argv[2]);

    if (max_threads < 1 || array_size < 1) {
        fprintf(stderr, "Invalid arguments\n");
        return EXIT_FAILURE;
    }

    uint32_t *a = malloc(array_size * sizeof *a);
    if (!a) {
        perror("malloc array");
        return EXIT_FAILURE;
    }

    FILE *f = fopen("../random_integers.bin", "rb");
    if (!f || fread(a, sizeof *a, array_size, f) != array_size) {
        perror("reading input");
        return EXIT_FAILURE;
    }
    fclose(f);

    omp_set_num_threads(max_threads);
    parallel_merge_sort_init(array_size);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    #pragma omp parallel
    {
        #pragma omp single nowait
        parallel_merge_sort_omp(a, 0, array_size);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec)
                + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (array_size / secs) / 1e6;
    printf("%zu,%zu,%.6f,%.3f\n", 
            max_threads, array_size, secs, mips);


    parallel_merge_sort_fini();
    free(a);
    return EXIT_SUCCESS;
}

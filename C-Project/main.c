#include <stdio.h>
#include <stdlib.h>
<<<<<<< HEAD
#include <stdint.h>
#include <time.h>
#include <omp.h>
#include "merge_sort.h"
=======
#include <string.h>
#include <time.h>
#include <uv.h>
>>>>>>> origin/main

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <maxThreads> <arraySize>\n", argv[0]);
        return EXIT_FAILURE;
    }
<<<<<<< HEAD
    int max_threads = atoi(argv[1]);
    size_t n = (size_t)atoll(argv[2]);
    if (max_threads < 1 || n < 1) {
=======

    size_t max_threads = atoi(argv[1]);
    size_t array_size = atoi(argv[2]);

    if (max_threads < 1 || array_size < 1) {
>>>>>>> origin/main
        fprintf(stderr, "Invalid arguments\n");
        return EXIT_FAILURE;
    }

<<<<<<< HEAD
    uint32_t *a = malloc(n * sizeof *a);
    if (!a) {
        perror("malloc array");
        return EXIT_FAILURE;
    }

    FILE *f = fopen("../random_integers.bin", "rb");
    if (!f || fread(a, sizeof *a, n, f) != n) {
        perror("reading input");
        return EXIT_FAILURE;
    }
    fclose(f);

    omp_set_num_threads(max_threads);
    parallel_merge_sort_init(n);
=======
    // 1) Control libuv thread‑pool size
    char envbuf[64];
    snprintf(envbuf, sizeof(envbuf), "UV_THREADPOOL_SIZE=%zu", max_threads);
    putenv(strdup(envbuf)); // strdup so memory remains valid

    // 2) Read exactly arraySize integers
    const char *in_path = "../random_integers.bin";
    FILE *file = fopen(in_path, "r");
    if (!file) {
        perror("Expected to find file random_integers.bin");
        return 1;
    }
    uint32_t *integers = malloc(array_size * sizeof(uint32_t));
    size_t read_count = fread(integers, sizeof(uint32_t), array_size, file);
    if (read_count != array_size) {
        fprintf(stderr, "Size mismatch: need %zu ints, got %zu\n", array_size, read_count);
        free(integers);
        fclose(file);
        return 1;
    }
    fclose(file);

    // 3) Prepare libuv loop and sync primitives
    uv_loop_t *loop = uv_default_loop();
    uv_mutex_t mutex;
    uv_mutex_init(&mutex);
>>>>>>> origin/main

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

<<<<<<< HEAD
    #pragma omp parallel
    {
        #pragma omp single nowait
        parallel_merge_sort_omp(a, 0, n);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec)
                + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (n / secs) / 1e6;
    printf("%d,%zu,%.6f,%.3f\n",
           max_threads, n, secs, mips);

    parallel_merge_sort_fini();
    free(a);
    return EXIT_SUCCESS;
=======
    merge_sort_uv(integers, 0, array_size - 1, loop, &mutex);
    uv_run(loop, UV_RUN_DEFAULT);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    uint32_t last = 0;
    for (size_t i = 0; i < array_size; i++) {
        if (integers[i] < last) {
            fprintf(stderr, "Array not sorted at index %zu: %u < %u\n", i, integers[i], last);
            free(integers);
            return 1;
        }
        last = integers[i];
    }

    double secs = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (array_size / secs) / 1e6;

    // 5) Print to stdout (CSV line)
    //    maxThreads,arraySize,seconds,MIps
    printf("%zu,%zu,%.6f,%.3f\n", max_threads, array_size, secs, mips);

    // cleanup
    uv_mutex_destroy(&mutex);
    free(integers);
    return 0;
>>>>>>> origin/main
}

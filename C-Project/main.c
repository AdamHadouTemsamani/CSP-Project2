#include "merge_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uv.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <maxThreads> <arraySize>\n", argv[0]);
        return 1;
    }

    size_t max_threads = atoi(argv[1]);
    size_t array_size = atoi(argv[2]);

    if (max_threads < 1 || array_size < 1) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    // 1) Control libuv thread‑pool size
    char envbuf[64];
    snprintf(envbuf, sizeof(envbuf), "UV_THREADPOOL_SIZE=%zu", max_threads);
    putenv(strdup(envbuf)); // strdup so memory remains valid

    // 2) Read exactly arraySize integers
    const char *in_path = "../random_integers.bin";
    FILE *file = fopen(in_path, "r");
    if (!file) {
        perror("Expected to find file random_integers.bin");
        fclose(file);
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

    // 4) Time the parallel merge‑sort
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    merge_sort_uv(integers, 0, array_size - 1, loop, &mutex);
    uv_run(loop, UV_RUN_DEFAULT);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (array_size / secs) / 1e6;

    // 5) Print to stdout (CSV line)
    //    maxThreads,arraySize,seconds,MIps
    printf("%zu,%zu,%.6f,%.3f\n", max_threads, array_size, secs, mips);

    // cleanup
    uv_mutex_destroy(&mutex);
    free(integers);
    return 0;
}

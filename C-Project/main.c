#include "merge_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <uv.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <maxThreads> <arraySize>\n", argv[0]);
        return 1;
    }
    int maxThreads  = atoi(argv[1]);
    int arraySize   = atoi(argv[2]);
    if (maxThreads < 1 || arraySize < 1) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    // 1) Control libuv thread‑pool size
    char envbuf[32];
    snprintf(envbuf, sizeof(envbuf), "UV_THREADPOOL_SIZE=%d", maxThreads);
    putenv(strdup(envbuf));  // strdup so memory remains valid

    // 2) Read exactly arraySize integers
    const char *in_path = "../random_int_list.txt";
    FILE *fin = fopen(in_path, "r");
    if (!fin) {
        perror("fopen input file");
        return 1;
    }
    int *data = malloc(arraySize * sizeof(int));
    for (int i = 0; i < arraySize; i++) {
        if (fscanf(fin, "%d\n", &data[i]) != 1) {
            fprintf(stderr, "Size mismatch: need %d ints, got %d\n",
                    arraySize, i);
            return 1;
        }
    }
    fclose(fin);

    // 3) Prepare libuv loop and sync primitives
    uv_loop_t *loop = uv_default_loop();
    uv_mutex_t mutex;
    uv_mutex_init(&mutex);
    int counter = 0;

    // 4) Time the parallel merge‑sort
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    merge_sort_uv(data, 0, arraySize - 1, loop, &mutex, &counter);
    uv_run(loop, UV_RUN_DEFAULT);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec)
                + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (arraySize / secs) / 1e6;

    // 5) Print to stdout (CSV line)
    //    maxThreads,arraySize,seconds,MIps
    printf("%d,%d,%.6f,%.3f\n",
           maxThreads, arraySize, secs, mips);

    // cleanup
    uv_mutex_destroy(&mutex);
    free(data);
    return 0;
}

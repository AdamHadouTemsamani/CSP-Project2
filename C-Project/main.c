#include "merge_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **read_all_lines(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) { perror("open"); return NULL; }
    size_t cap = 8, n = 0;
    char **lines = malloc(cap * sizeof(char*));
    char buf[256];
    while (fgets(buf, sizeof(buf), file)) {
        if (n >= cap - 1) {
            cap *= 2;
            lines = realloc(lines, cap * sizeof(char*));
        }
        lines[n++] = strdup(buf);
    }
    lines[n] = NULL;
    fclose(file);
    return lines;
}

// Now returns both the array and its length
int *str_to_ints(char **strs, size_t *out_count) {
    size_t cap = 8, n = 0;
    int *a = malloc(cap * sizeof(int));
    for (size_t i = 0; strs[i]; i++) {
        if (n >= cap) {
            cap *= 2;
            a = realloc(a, cap * sizeof(int));
        }
        a[n++] = atoi(strs[i]);
    }
    *out_count = n;
    return a;
}

int main(int argc, char *argv[]) {
    // Set thread-pool size for libuv
    putenv("UV_THREADPOOL_SIZE=8");

    // Read and parse input
    char **lines = read_all_lines("../random_int_list.txt");
    if (!lines) return 1;

    size_t count;
    int *ints = str_to_ints(lines, &count);

    // Prepare libuv loop + sync
    uv_loop_t *loop = uv_default_loop();
    uv_mutex_t mutex;
    uv_mutex_init(&mutex);
    int counter = 0;

    // Kick off parallel merge sort
    merge_sort_uv(ints, 0, (int)count - 1, loop, &mutex, &counter);

    // Run until all work is done
    uv_run(loop, UV_RUN_DEFAULT);

    // Print sorted results
    for (size_t i = 0; i < count; i++) {
        printf("%d\n", ints[i]);
    }

    uv_mutex_destroy(&mutex);
    return 0;
}

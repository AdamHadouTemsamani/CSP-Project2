#include "merge_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **read_all_lines(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }
    char **lines = NULL;
    size_t line_count = 0;
    size_t line_capacity = 8;
    lines = malloc(line_capacity * sizeof(char *));
    if (!lines) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }
    int buffer_size = 256;
    char buffer[buffer_size];
    while (fgets(buffer, buffer_size, file)) {
        // Minus one because we should always have a NULL at the end of the list.
        if (line_count >= line_capacity - 1) {
            line_capacity *= 2;
            lines = realloc(lines, line_capacity * sizeof(char *));
            if (!lines) {
                perror("Failed to reallocate memory");
                fclose(file);
                return NULL;
            }
        }
        lines[line_count] = strdup(buffer);
        if (!lines[line_count]) {
            perror("Failed to duplicate string");
            fclose(file);
            return NULL;
        }
        line_count++;
    }
    lines[line_count] = NULL;
    fclose(file);
    return lines;
}

int *str_to_ints(char **strs) {
    int *ints = NULL;
    size_t int_count = 0;
    size_t int_capacity = 8;
    ints = malloc(int_capacity * sizeof(int));
    if (!ints) {
        perror("Failed to allocate memory");
        return NULL;
    }
    for (size_t i = 0; strs[i] != NULL; i++) {
        if (int_count >= int_capacity - 1) {
            int_capacity *= 2;
            ints = realloc(ints, int_capacity * sizeof(int));
            if (!ints) {
                perror("Failed to reallocate memory");
                return NULL;
            }
        }
        ints[int_count++] = atoi(strs[i]);
    }
    ints[int_count] = 0;
    return ints;
}

int main(int argc, char *argv[]) {
    int max_threads = 8;
    char threadpool_size_env[32];
    sprintf(threadpool_size_env, "UV_THREADPOOL_SIZE=%d", max_threads);
    putenv(threadpool_size_env);
    char **strs = read_all_lines("../random_int_list.txt");
    int *ints = str_to_ints(strs);
    int count = 0;
    while (ints[count] != 0) {
        count++;
    }
    merge_sort(ints, 0, count - 1);
    for (int i = 0; i < count; i++) {
        printf("%d\n", ints[i]);
    }
    return 0;
}

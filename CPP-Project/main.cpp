#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <tbb/global_control.h>
#include "merge_sort.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::fprintf(stderr, "Usage: %s <maxThreads> <arraySize>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int max_threads = std::atoi(argv[1]);
    size_t n        = std::strtoull(argv[2], nullptr, 10);
    if (max_threads < 1 || n < 1) {
        std::fprintf(stderr, "Invalid arguments\n");
        return EXIT_FAILURE;
    }

    // Limit TBB parallelism
    tbb::global_control control(tbb::global_control::max_allowed_parallelism, max_threads);

    // Read binary file of little-endian uint32_t
    const char *path = "random_integers.bin";
    FILE *f = std::fopen(path, "rb");
    if (!f) { perror("fopen"); return EXIT_FAILURE; }
    uint32_t *a = (uint32_t*)std::malloc(n * sizeof *a);
    if (!a) { perror("malloc"); std::fclose(f); return EXIT_FAILURE; }
    if (std::fread(a, sizeof *a, n, f) != n) {
        std::fprintf(stderr, "Expected %zu ints but got fewer\n", n);
        std::fclose(f);
        std::free(a);
        return EXIT_FAILURE;
    }
    std::fclose(f);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    parallel_merge_sort(a, n);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec)
                + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    double mips = (n / secs) / 1e6;
    std::printf("steady,%d,%.0f,%.6f,%.3f\n",
            max_threads, (double)n, secs, mips);

    for (size_t i = 0; i < n; ++i) {
        if (i > 0 && a[i] < a[i-1]) {
            std::fprintf(stderr, "Array not sorted at index %zu\n", i);
            std::free(a);
            return EXIT_FAILURE;
        }
    }        

    std::free(a);
    return EXIT_SUCCESS;
}

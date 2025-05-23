# CSP-Project2

Computer Systems Performance - Project 2 (Comparison of Programming Languages for Parallel Merge Sort)

## Running the project

1. Generate the random_integers.bin file with `echo 67108864 | python generate_random_ints.py`
2. Then run the experiment using command `make clean && make all`
3. If visualization of the results is desired run `python scripts/visualize_gc.py`, `python scripts/visualize_perf.py`,
and `python scripts/visualize_throughput.py`
4. If step 3 fails due to missing dependencies install them and go back to step 3

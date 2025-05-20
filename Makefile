SHELL := /bin/bash

# -----------------------------------------------------------------------------
# Compiler/tool definitions
# -----------------------------------------------------------------------------
CC       := gcc
CFLAGS   := -O2 -Wall -pthread -fopenmp
LDFLAGS  :=

DOTNET   := dotnet
CSPROJ   := CSharp-Project/MergeSortPerf/MergeSortPerf.csproj

CARGO    := cargo
CARGO_TGT:= Rust-Project/merge_sort_perf

# -----------------------------------------------------------------------------
# Directories & output files
# -----------------------------------------------------------------------------
BUILD_DIR    := build
RESULTS_DIR  := results
PERF_DIR     := perf

# throughput files per language
TH_C         := $(RESULTS_DIR)/throughput_c.csv
TH_CS        := $(RESULTS_DIR)/throughput_cs.csv
TH_RS        := $(RESULTS_DIR)/throughput_rs.csv

# perf files per language
PF_C         := $(PERF_DIR)/perf_c.txt
PF_CS        := $(PERF_DIR)/perf_cs.txt
PF_RS        := $(PERF_DIR)/perf_rs.txt

# -----------------------------------------------------------------------------
# Experiment parameters
# -----------------------------------------------------------------------------
THREADS := 1 2 4 8 16
SIZES   := 10 100 1000 10000 100000 1000000
REPEAT  := 3

# perf events
EVENTS := cpu-cycles,cache-misses,dTLB-load-misses,context-switches

# -----------------------------------------------------------------------------
# Phony targets
# -----------------------------------------------------------------------------
.PHONY: all clean prepare_dirs build_c build_cs build_rs init_outputs run

all: prepare_dirs build_c build_cs build_rs init_outputs run

clean:
	rm -rf $(BUILD_DIR) $(RESULTS_DIR) $(PERF_DIR)
	-$(DOTNET) clean $(CSPROJ)
	cd $(CARGO_TGT) && $(CARGO) clean

prepare_dirs:
	mkdir -p $(BUILD_DIR) $(RESULTS_DIR) $(PERF_DIR)

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------
build_c:
	$(CC) $(CFLAGS) \
	  C-Project/main.c \
	  C-Project/merge_sort.c \
	  -o $(BUILD_DIR)/parallel_merge_sort_c \
	  $(LDFLAGS)

build_cs:
	$(DOTNET) build $(CSPROJ) -c Release -o ../$(BUILD_DIR)/parallel_merge_sort_cs

build_rs:
	cd $(CARGO_TGT) && $(CARGO) build --release
	ln -f $(CARGO_TGT)/target/release/merge_sort_perf \
	      $(BUILD_DIR)/parallel_merge_sort_rs

# -----------------------------------------------------------------------------
# Initialize all throughput & perf outputs (empty files with headers)
# -----------------------------------------------------------------------------
init_outputs:
	@echo "maxThreads,arraySize,seconds,MIps" > $(TH_C)
	@echo "maxThreads,arraySize,seconds,MIps" > $(TH_CS)
	@echo "maxThreads,arraySize,seconds,MIps" > $(TH_RS)
	@echo "# perf log for C"      > $(PF_C)
	@echo "# perf log for C#"     > $(PF_CS)
	@echo "# perf log for Rust"   > $(PF_RS)

# -----------------------------------------------------------------------------
# Run experiments
# -----------------------------------------------------------------------------
run:
	@for lang in c cs rs; do \
	  bin=$(BUILD_DIR)/parallel_merge_sort_$$lang; \
	  th_file=$(RESULTS_DIR)/throughput_$$lang.csv; \
	  pf_file=$(PERF_DIR)/perf_$$lang.txt; \
	  echo ">>> Running $$lang: logging to $$th_file and $$pf_file <<<"; \
	  for t in $(THREADS); do \
	    for s in $(SIZES); do \
	      echo "-- threads=$$t size=$$s --" >> $$pf_file; \
	      for rep in $$(seq 1 $(REPEAT)); do \
	        echo "[run $$rep]:" >> $$pf_file; \
	        { /usr/bin/perf stat -e $(EVENTS) \
	            $$bin $$t $$s > /dev/null; } 2>> $$pf_file; \
	      done; \
	      # extract the *average* elapsed seconds from the perf output:
	      secs=$$(grep -Po '(?<=seconds time elapsed\W)\d+\.\d+' $$pf_file \
	              | tail -n $(REPEAT) \
	              | awk '{sum+=$$1} END{printf "%.6f", sum/NR}'); \
	      # fallback if no perf timestamps:
	      if [ -z "$$secs" ]; then \
	        secs=$$( $$bin $$t $$s | awk -F, '{print $$3}' ); \
	      fi; \
	      mips=$$(awk "BEGIN{printf \"%.3f\", ($$s/$$secs)/1e6}"); \
	      echo "$$t,$$s,$$secs,$$mips" >> $$th_file; \
	    done; \
	  done; \
	done

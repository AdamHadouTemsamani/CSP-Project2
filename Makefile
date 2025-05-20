SHELL := /bin/bash

# -----------------------------------------------------------------------------
# Compiler/tool definitions
# -----------------------------------------------------------------------------
CC       := gcc
CFLAGS   := -Wall -Wextra -O3 -g -fopenmp
LDFLAGS  := -fopenmp

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

TH_C         := $(RESULTS_DIR)/throughput_c.csv
TH_CS        := $(RESULTS_DIR)/throughput_cs.csv
TH_RS        := $(RESULTS_DIR)/throughput_rs.csv

PF_C         := $(PERF_DIR)/perf_c.txt
PF_CS        := $(PERF_DIR)/perf_cs.txt
PF_RS        := $(PERF_DIR)/perf_rs.txt

# -----------------------------------------------------------------------------
# Experiment parameters
# -----------------------------------------------------------------------------
THREADS := 1 2 4 8 16
SIZES   := 10 100 1000 10000 100000 1000000
REPEAT  := 3

EVENTS  := cpu-cycles,cache-misses,dTLB-load-misses,context-switches

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
	  C-Project/main.c C-Project/merge_sort.c \
	  -o $(BUILD_DIR)/parallel_merge_sort_c $(LDFLAGS)

build_cs:
	# Publish C# as a DLL into build/
	$(DOTNET) publish $(CSPROJ) \
	    -c Release \
	    -r linux-x64 \
	    --self-contained false \
	    -o $(BUILD_DIR)
	# Rename for clarity
	mv $(BUILD_DIR)/MergeSortPerf.dll \
	   $(BUILD_DIR)/parallel_merge_sort_cs.dll

build_rs:
	cd $(CARGO_TGT) && $(CARGO) build --release \
	  && ln -f target/release/merge_sort_perf \
	         $(abspath $(BUILD_DIR)/parallel_merge_sort_rs)

# -----------------------------------------------------------------------------
# Initialize CSV/perf outputs
# -----------------------------------------------------------------------------
init_outputs:
	@echo "maxThreads,arraySize,seconds,MIps" > $(TH_C)
	@echo "maxThreads,arraySize,seconds,MIps" > $(TH_CS)
	@echo "maxThreads,arraySize,seconds,MIps" > $(TH_RS)
	@echo "# perf log for C"    > $(PF_C)
	@echo "# perf log for C#"   > $(PF_CS)
	@echo "# perf log for Rust" > $(PF_RS)

# -----------------------------------------------------------------------------
# Run experiments
# -----------------------------------------------------------------------------
run:
	for lang in c cs rs; do \
	  # pick the right binary/dll \
	  if [ "$$lang" = "cs" ]; then \
	    bin="$(BUILD_DIR)/parallel_merge_sort_cs.dll"; \
	  else \
	    bin="$(BUILD_DIR)/parallel_merge_sort_$$lang"; \
	  fi; \
	  if [ ! -f $$bin ]; then \
	    echo "[!] Skipping $$lang"; continue; \
	  fi; \
	  th_file=$(RESULTS_DIR)/throughput_$$lang.csv; \
	  pf_file=$(PERF_DIR)/perf_$$lang.txt; \
	  echo ">>> Running $$lang <<<"; \
	  for t in $(THREADS); do \
	    for s in $(SIZES); do \
	      echo "-- threads=$$t size=$$s --" >> $$pf_file; \
	      for rep in $$(seq 1 $(REPEAT)); do \
	        echo "[run $$rep]:" >> $$pf_file; \
	        if [ "$$lang" = "cs" ]; then \
	          { /usr/bin/perf stat -e $(EVENTS) dotnet $$bin $$t $$s > /dev/null; } 2>> $$pf_file; \
	        else \
	          { /usr/bin/perf stat -e $(EVENTS) $$bin $$t $$s > /dev/null; } 2>> $$pf_file; \
	        fi; \
	      done; \
	      raw=$$(grep -Po '(?<=seconds time elapsed\W)\d+\.\d+' $$pf_file | tail -n $(REPEAT)); \
	      if [ -n "$$raw" ]; then \
	        secs=$$(echo "$$raw" | awk '{sum+=$$1} END{printf "%.6f", sum/NR}'); \
	      else \
	        if [ "$$lang" = "cs" ]; then \
	          secs=$$( dotnet $$bin $$t $$s | awk -F, '{print $$3}' ); \
	        else \
	          secs=$$( $$bin $$t $$s | awk -F, '{print $$3}' ); \
	        fi; \
	      fi; \
	      mips=$$(awk "BEGIN{printf \"%.3f\", ($$s/$$secs)/1e6}"); \
	      echo "$$t,$$s,$$secs,$$mips" >> $$th_file; \
	    done; \
	  done; \
	done

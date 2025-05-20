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
BUILD_DIR   := build
RESULTS_DIR := results
PERF_DIR    := perf

TH_C  := $(RESULTS_DIR)/throughput_c.csv
TH_CS := $(RESULTS_DIR)/throughput_cs.csv
TH_RS := $(RESULTS_DIR)/throughput_rs.csv

PF_C  := $(PERF_DIR)/perf_c.txt
PF_CS := $(PERF_DIR)/perf_cs.txt
PF_RS := $(PERF_DIR)/perf_rs.txt

# -----------------------------------------------------------------------------
# Experiment parameters
# -----------------------------------------------------------------------------
THREADS := 1 2 4 8 16
SIZES   := 10 100 1000 10000 100000 1000000
REPEAT  := 3
EVENTS  := cpu-cycles,cache-misses,dTLB-load-misses,context-switches

.PHONY: all clean prepare_dirs build_c build_cs build_rs init_outputs run

all: prepare_dirs build_c build_cs build_rs init_outputs run

clean:
	@rm -rf $(BUILD_DIR) $(RESULTS_DIR) $(PERF_DIR)
	-@$(DOTNET) clean $(CSPROJ)
	@cd $(CARGO_TGT) && $(CARGO) clean

prepare_dirs:
	mkdir -p $(BUILD_DIR) $(RESULTS_DIR) $(PERF_DIR)

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------
build_c:
	$(CC) $(CFLAGS) C-Project/main.c C-Project/merge_sort.c \
	  -o $(BUILD_DIR)/parallel_merge_sort_c $(LDFLAGS)

build_cs:
	$(DOTNET) publish $(CSPROJ) -c Release --no-self-contained -o $(BUILD_DIR)
	ln -f $(BUILD_DIR)/MergeSortPerf.dll \
	      $(BUILD_DIR)/parallel_merge_sort_cs.dll
	cp $(BUILD_DIR)/MergeSortPerf.runtimeconfig.json \
	   $(BUILD_DIR)/parallel_merge_sort_cs.runtimeconfig.json
	cp $(BUILD_DIR)/MergeSortPerf.deps.json \
	   $(BUILD_DIR)/parallel_merge_sort_cs.deps.json

build_rs:
	cd $(CARGO_TGT) && $(CARGO) build --release \
	  && ln -f target/release/merge_sort_perf \
	         $(abspath $(BUILD_DIR)/parallel_merge_sort_rs)

# -----------------------------------------------------------------------------
# Initialize CSV & perf logs
# -----------------------------------------------------------------------------
init_outputs:
	@echo "phase,threads,size,seconds,mips" > $(TH_C)
	@echo "phase,threads,size,seconds,mips" > $(TH_CS)
	@echo "phase,threads,size,seconds,mips" > $(TH_RS)
	@echo "# perf log for C"    > $(PF_C)
	@echo "# perf log for C#"   > $(PF_CS)
	@echo "# perf log for Rust" > $(PF_RS)

# -----------------------------------------------------------------------------
# Run experiments (stdout preserved)
# -----------------------------------------------------------------------------
run:
	for lang in c cs rs; do \
	  if   [ "$$lang" = "c"  ]; then \
	    run_cmd="$(BUILD_DIR)/parallel_merge_sort_c"; \
	    th_file="$(TH_C)";  pf_file="$(PF_C)"; \
	  elif [ "$$lang" = "cs" ]; then \
	    run_cmd="dotnet $(BUILD_DIR)/parallel_merge_sort_cs.dll"; \
	    th_file="$(TH_CS)"; pf_file="$(PF_CS)"; \
	  else \
	    run_cmd="$(BUILD_DIR)/parallel_merge_sort_rs"; \
	    th_file="$(TH_RS)"; pf_file="$(PF_RS)"; \
	  fi; \
	  if [ ! -f $$(echo $$run_cmd | awk '{print $$2}') ]; then \
	    echo "[!] Skipping $$lang"; \
	    continue; \
	  fi; \
	  echo ">>> Running $$lang <<<"; \
	  for t in $(THREADS); do \
	    for s in $(SIZES); do \
	      echo "-- threads=$$t size=$$s --" >> "$$pf_file"; \
	      for rep in $$(seq 1 $(REPEAT)); do \
	        echo "[run $$rep]:" >> "$$pf_file"; \
	        /usr/bin/perf stat -e $(EVENTS) $$run_cmd $$t $$s \
	          2>>"$$pf_file" \
	          | tee -a "$$th_file"; \
	      done; \
	    done; \
	  done; \
	done

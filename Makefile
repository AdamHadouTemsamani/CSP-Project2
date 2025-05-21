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

# Throughput CSVs
TH_C         := $(RESULTS_DIR)/throughput_c.csv
TH_CS        := $(RESULTS_DIR)/throughput_cs.csv
TH_CS_GC     := $(RESULTS_DIR)/throughput_cs_gc.csv
TH_RS        := $(RESULTS_DIR)/throughput_rs.csv

# Perf logs
PF_C         := $(PERF_DIR)/perf_c.txt
PF_CS        := $(PERF_DIR)/perf_cs.txt
PF_CS_GC     := $(PERF_DIR)/perf_cs_gc.txt
PF_RS        := $(PERF_DIR)/perf_rs.txt

# GC runtime counters CSV
GC_CS        := $(PERF_DIR)/gc_cs.csv

# -----------------------------------------------------------------------------
# Experiment parameters
# -----------------------------------------------------------------------------
THREADS := 1 2 4 8 16 32
SIZES   := 256 1024 4096 16384 65536 262144 1048576 4194304 16777216
REPEAT  := 5
EVENTS  := cpu-cycles,instructions,cache-misses,LLC-load-misses,\
            dTLB-load-misses,branch-misses,context-switches

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
	@echo "phase,threads,size,seconds,mips"      > $(TH_C)
	@echo "phase,threads,size,seconds,mips"      > $(TH_CS)
	@echo "phase,threads,size,seconds,mips"      > $(TH_CS_GC)
	@echo "phase,threads,size,seconds,mips"      > $(TH_RS)

	@echo "# perf log for C"                     > $(PF_C)
	@echo "# perf log for C#"                    > $(PF_CS)
	@echo "# perf log for C# (GC)"               > $(PF_CS_GC)
	@echo "# perf log for Rust"                  > $(PF_RS)

	@echo "Timestamp,CounterName,Value"          > $(GC_CS)

# -----------------------------------------------------------------------------
# Run experiments (stdout preserved)
# -----------------------------------------------------------------------------
run:
	for lang in c cs rs; do \
	  if   [ "$$lang" = "c"  ]; then \
	    run_cmd="$(BUILD_DIR)/parallel_merge_sort_c"; \
	    th_file="$(TH_C)"; pf_file="$(PF_C)"; \
	  elif [ "$$lang" = "cs" ]; then \
	    run_cmd="dotnet $(BUILD_DIR)/parallel_merge_sort_cs.dll"; \
	    th_file="$(TH_CS)"; pf_file="$(PF_CS)"; \
	    th_gc_file="$(TH_CS_GC)"; pf_gc_file="$(PF_CS_GC)"; gc_file="$(GC_CS)"; \
	  else \
	    run_cmd="$(BUILD_DIR)/parallel_merge_sort_rs"; \
	    th_file="$(TH_RS)"; pf_file="$(PF_RS)"; \
	  fi; \
\
	  # existence check
	  if ! command -v $$(echo $$run_cmd | awk '{print $$1}') >/dev/null 2>&1 \
	     && [ ! -x "$$run_cmd" ]; then \
	    echo "[!] Skipping $$lang (not found or not executable)"; \
	    continue; \
	  fi; \
\
	  echo ">>> Running $$lang <<<"; \
	  for t in $(THREADS); do \
	    for s in $(SIZES); do \
	      echo "-- threads=$$t size=$$s --" >> "$$pf_file"; \
	      for rep in $$(seq 1 $(REPEAT)); do \
	        echo "[run $$rep]:" >> "$$pf_file"; \
\
	        # perf + throughput
	        /usr/bin/perf stat -e $(EVENTS) $$run_cmd $$t $$s \
	          2>>"$$pf_file" \
	          | tee -a "$$th_file"; \
\
	        # and GC/second pass for C# only
	        if [ "$$lang" = "cs" ]; then \
	          echo "[run $$rep]:" >> "$$pf_gc_file"; \
	          /usr/bin/perf stat -e $(EVENTS) dotnet-counters collect \
	            --format csv \
	            --counters System.Runtime \
	            --refresh-interval 1 \
	            --output "$$gc_file" \
	            -- $$run_cmd $$t $$s \
	            2>>"$$pf_gc_file" \
	            | tee -a "$$th_gc_file"; \
	        fi; \
\
	      done; \
	    done; \
	  done; \
	done

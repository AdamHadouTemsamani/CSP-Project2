SHELL := /bin/bash

# -----------------------------------------------------------------------------
# Compiler/tool definitions
# -----------------------------------------------------------------------------
CXX       := g++
CXXFLAGS  := -Wall -Wextra -O3 -g -std=c++17
LDLIBS    := -ltbb

DOTNET    := dotnet
CSPROJ    := CSharp-Project/MergeSortPerf/MergeSortPerf.csproj

CARGO     := cargo
CARGO_TGT := Rust-Project/merge_sort_perf

PERF      := perf

# -----------------------------------------------------------------------------
# Directories & output files
# -----------------------------------------------------------------------------
BUILD_DIR   := build
RESULTS_DIR := results
PERF_DIR    := perf
GC_DIR      := perf/gc

THROUGHPUT_CPP   := $(RESULTS_DIR)/throughput_cpp.csv
THROUGHPUT_CS    := $(RESULTS_DIR)/throughput_cs.csv
THROUGHPUT_RS    := $(RESULTS_DIR)/throughput_rs.csv

PERF_CPP   := $(PERF_DIR)/perf_cpp.txt
PERF_CS    := $(PERF_DIR)/perf_cs.txt
PERF_RS    := $(PERF_DIR)/perf_rs.txt

GC_CS := $(GC_DIR)/gc_cs

# -----------------------------------------------------------------------------
# Experiment parameters
# -----------------------------------------------------------------------------
THREADS := 1 2 4 8 16 32
SIZES   := 4096 16384 65536 262144 1048576 4194304 16777216 67108864
REPEAT  := 3
EVENTS  := cpu-cycles,instructions,cache-misses,LLC-load-misses,dTLB-load-misses,branch-misses,context-switches

.PHONY: all clean prepare_dirs build_cpp build_cs build_rs init_outputs run

all: prepare_dirs build_cpp build_cs build_rs init_outputs run

clean:
	rm -rf $(BUILD_DIR) $(RESULTS_DIR) $(PERF_DIR) $(GC_DIR)
	$(DOTNET) clean $(CSPROJ)
	cd $(CARGO_TGT) && $(CARGO) clean

prepare_dirs:
	mkdir -p $(BUILD_DIR) $(RESULTS_DIR) $(PERF_DIR) $(GC_DIR)

# -----------------------------------------------------------------------------
# Build rules
# -----------------------------------------------------------------------------
build_cpp:
	$(CXX) $(CXXFLAGS) CPP-Project/main.cpp CPP-Project/merge_sort.cpp \
	    -o $(BUILD_DIR)/parallel_merge_sort_cpp $(LDLIBS)

build_cs:
	$(DOTNET) publish $(CSPROJ) \
	    -c Release --no-self-contained \
	    -o $(BUILD_DIR)
	ln -f $(BUILD_DIR)/MergeSortPerf $(BUILD_DIR)/parallel_merge_sort_cs

build_rs:
	cd $(CARGO_TGT) && $(CARGO) build --release
	ln -f $(CARGO_TGT)/target/release/merge_sort_perf \
	      $(BUILD_DIR)/parallel_merge_sort_rs

# -----------------------------------------------------------------------------
# Initialize CSV & perf logs
# -----------------------------------------------------------------------------
init_outputs:
	@echo "phase,threads,size,seconds,mips" > $(THROUGHPUT_CPP)
	@echo "phase,threads,size,seconds,mips" > $(THROUGHPUT_CS)
	@echo "phase,threads,size,seconds,mips" > $(THROUGHPUT_RS)
	@echo "# perf log for C"                > $(PERF_CPP)
	@echo "# perf log for C#"               > $(PERF_CS)
	@echo "# perf log for Rust"             > $(PERF_RS)
	@echo "timestamp,counter_name,value"    > $(GC_CS)

# -----------------------------------------------------------------------------
# Run experiments
# -----------------------------------------------------------------------------
run:
	set -euo pipefail
	for lang in cpp rs cs; do \
		if [ "$$lang" = "cpp" ]; then \
			bin="$(BUILD_DIR)/parallel_merge_sort_cpp"; \
			throughput="$(THROUGHPUT_CPP)"; \
			perf="$(PERF_CPP)"; \
		elif [ "$$lang" = "rs" ]; then \
			bin="$(BUILD_DIR)/parallel_merge_sort_rs"; \
			throughput="$(THROUGHPUT_RS)"; \
			perf="$(PERF_RS)"; \
		else \
			bin="$(BUILD_DIR)/parallel_merge_sort_cs"; \
			throughput="$(THROUGHPUT_CS)"; \
			perf="$(PERF_CS)"; \
			throughput_gc="$(THROUGHPUT_CS)"; \
			perf_gc="$(PERF_CS)"; \
			gc="$(GC_CS)"; \
		fi; \
		if [ ! -x $$bin ]; then \
			echo "[!] Skipping $lang (no binary)"; \
			continue; \
		fi; \
		echo ">>> Running $$lang <<<"; \
		for threads in $(THREADS); do \
			for size in $(SIZES); do \
				echo "-- threads=$$threads size=$$size --" >> $$perf; \
				for rep in $$(seq 1 $(REPEAT)); do \
					echo "[run $$rep]:" >> $$perf; \
					$(PERF) stat -e $(EVENTS) $$bin $$threads $$size \
						2>>"$$perf" \
						| tee -a $$throughput; \
					if [ "$$lang" = "cs" ]; then \
						echo "[run $$rep]:" >> "$$perf_gc"; \
						dotnet-counters collect \
							--format csv \
							--counters System.Runtime \
							--output $${gc}_$${rep}_$${threads}_$${size}.csv \
							-- $$bin $$threads $$size skipSteady \
							2>>"$$perf_gc" \
							| tee -a $$throughput_gc; \
					fi; \
				done; \
			done; \
		done; \
	done


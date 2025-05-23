#!/usr/bin/env python3
import os
import glob
import re
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import math

PERF_DIR = "perf"
OUT_DIR  = "images/perf"
os.makedirs(OUT_DIR, exist_ok=True)

def parse_perf_file(path):
    records = []
    current_threads = None
    current_size = None
    with open(path) as f:
        for line in f:
            line = line.strip()
            m = re.match(r"-- threads=(\d+)\s+size=(\d+)", line)
            if m:
                current_threads = int(m.group(1))
                current_size    = int(m.group(2))
                continue
            if line.startswith("Performance counter stats"):
                continue
            m2 = re.match(r"([\d,]+)\s+(\S+)", line)
            if m2 and current_threads is not None:
                val = float(m2.group(1).replace(",", ""))
                met = m2.group(2)
                records.append({
                    "threads": current_threads,
                    "size":    current_size,
                    "metric":  met,
                    "value":   val
                })
    return pd.DataFrame(records)

def plot_perf_for_file(txt_path):
    label = os.path.splitext(os.path.basename(txt_path))[0]
    df = parse_perf_file(txt_path)
    if df.empty:
        print(f"[perf] no data in {txt_path}")
        return

    metrics = sorted(m for m in df["metric"].unique() if m != "LLC-load-misses")
    ncols = 2
    nrows = math.ceil(len(metrics) / ncols)
    fig, axes = plt.subplots(nrows, ncols, figsize=(ncols*5, nrows*4))
    axes = axes.flatten()

    for idx, met in enumerate(metrics):
        ax = axes[idx]
        sub = df[df["metric"] == met]
        for thr in sorted(sub["threads"].unique()):
            grp = sub[sub["threads"] == thr] \
                  .groupby("size", as_index=False)["value"] \
                  .mean()
            ax.plot(grp["size"], grp["value"],
                    marker="o", label=f"{thr} threads")
        ax.set_xscale("log", base=2)
        ax.set_title(met)
        ax.set_xlabel("Input size")
        ax.set_ylabel("Value")
        ax.grid(True)
        ax.legend(fontsize="small")

    for j in range(idx+1, len(axes)):
        fig.delaxes(axes[j])

    fig.tight_layout(rect=[0, 0, 1, 1])

    out_path = os.path.join(OUT_DIR, f"{label}.svg")
    fig.savefig(out_path, format="svg")
    plt.close(fig)
    print(f"[perf] wrote {out_path}")

def main():
    perf_files = sorted(glob.glob(os.path.join(PERF_DIR, "*.txt")))
    if not perf_files:
        print("[perf] no .txt files found")
        return

    for path in perf_files:
        plot_perf_for_file(path)

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
import os
import glob
import math
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

RESULTS_DIR = "results"
OUT_DIR     = "images/throughput"
os.makedirs(OUT_DIR, exist_ok=True)

def read_and_aggregate(path):
    rows = []
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("Starting") or line.startswith("File saved") or line.startswith("phase"):
                continue
            parts = line.split(",")
            if len(parts) != 5:
                continue
            phase, threads, size, _, mips = parts
            rows.append({
                "phase":   phase,
                "threads": int(threads),
                "size":    int(size),
                "mips":    float(mips)
            })
    if not rows:
        return pd.DataFrame()
    df = pd.DataFrame(rows)
    return df.groupby(["phase","threads","size"], as_index=False)["mips"].mean()

def exponent_formatter(x, pos):
    """Turn a power-of-two x into '2^n'."""
    if x <= 0:
        return ""
    n = int(round(math.log2(x)))
    return f"2^{n}"

def plot_throughput_file(path):
    df = read_and_aggregate(path)
    label = os.path.splitext(os.path.basename(path))[0].replace("throughput_", "")
    if df.empty:
        print(f"[throughput] skipping {label} (no data)")
        return

    # Ensure 'cold' appears left of 'steady'
    phases = []
    if label == "cs":
        if "cold" in df["phase"].unique():
            phases.append("cold")
        if "steady" in df["phase"].unique():
            phases.append("steady")
    else:
        if "steady" in df["phase"].unique():
            phases.append("steady")
        else:
            print(f"[throughput] skipping {label} (no steady data)")
            return

    sizes = sorted(df["size"].unique())

    fig, axes = plt.subplots(1, len(phases), figsize=(6 * len(phases), 5), squeeze=False)
    axes = axes[0]

    for ax, phase in zip(axes, phases):
        sub = df[df["phase"] == phase].sort_values("size")
        if sub.empty:
            continue
        for thr in sorted(sub["threads"].unique()):
            grp = sub[sub["threads"] == thr]
            ax.plot(grp["size"], grp["mips"], marker="o", label=f"{thr} threads")

        ax.set_xscale("log", base=2)
        ax.set_xticks(sizes)
        ax.xaxis.set_major_formatter(FuncFormatter(exponent_formatter))

        ax.set_title(f"{label.upper()} {phase.capitalize()}")
        ax.set_xlabel("Input Size")
        ax.set_ylabel("MiPS")
        ax.grid(True, which="both", linestyle="--", linewidth=0.5)
        ax.legend(fontsize="small")

    fig.tight_layout()
    out_path = os.path.join(OUT_DIR, f"{label}.svg")
    fig.savefig(out_path, format="svg")
    plt.close(fig)
    print(f"[throughput] wrote {out_path}")

def main():
    csv_files = sorted(glob.glob(os.path.join(RESULTS_DIR, "throughput_*.csv")))
    if not csv_files:
        print("[throughput] no CSV files found in 'results/'")
        return

    for path in csv_files:
        plot_throughput_file(path)

if __name__ == "__main__":
    main()

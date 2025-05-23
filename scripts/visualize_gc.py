# Full script update: combine all the logic with the new heatmap integration and SVG outputs

import glob
import os
import re
import math

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Directories
GC_DIR = "perf/gc"
OUT_DIR = "images/gc"
os.makedirs(OUT_DIR, exist_ok=True)

def load_and_tag_gc():
    """Read all gc_cs_<rep>_<threads>_<size>.csv and tag rep, threads, size."""
    dfs = []
    for path in glob.glob(os.path.join(GC_DIR, "gc_cs_*.csv")):
        m = re.search(r"gc_cs_(\d+)_(\d+)_(\d+)\.csv$", path)
        if not m:
            continue
        rep, thr, sz = map(int, m.groups())
        df = pd.read_csv(path)
        df["rep"] = rep
        df["threads"] = thr
        df["size"] = sz
        dfs.append(df)
    return pd.concat(dfs, ignore_index=True) if dfs else pd.DataFrame()

def aggregate_pause(df):
    """Compute average total GC pause time per (threads, size)."""
    pause = df[df["Counter Name"] == "dotnet.gc.pause.time (s / 1 sec)"]
    if pause.empty:
        return pd.DataFrame()
    summed = pause.groupby(["threads", "size", "rep"], as_index=False)["Mean/Increment"].sum()
    agg = summed.groupby(["threads", "size"], as_index=False)["Mean/Increment"].mean()
    agg = agg.rename(columns={"Mean/Increment": "total_pause_s"})
    return agg

def plot_pause_grid(agg):
    """Plot a grid of bar charts for GC pause time per size."""
    if agg.empty:
        print("[gc] No pause data to plot.")
        return
    sizes = sorted(agg["size"].unique())
    n = len(sizes)
    cols = 3
    rows = math.ceil(n / cols)
    fig, axes = plt.subplots(rows, cols, figsize=(cols*4, rows*3), sharey=True)
    axes = axes.flatten()
    ymax = agg["total_pause_s"].max()
    ypad = ymax * 0.05
    for idx, sz in enumerate(sizes):
        ax = axes[idx]
        sub = agg[agg["size"] == sz].sort_values("threads")
        threads = sub["threads"].astype(str)
        pauses = sub["total_pause_s"]
        colors = plt.cm.viridis(sub["threads"] / sub["threads"].max())
        bars = ax.bar(threads, pauses, color=colors)
        ax.set_title(f"Size 2^{int(math.log2(sz))}")
        ax.set_xlabel("Threads")
        if idx % cols == 0:
            ax.set_ylabel("Total GC Pause (s)")
        ax.set_ylim(0, ymax + ypad)
        ax.grid(axis="y", linestyle="--", alpha=0.6)
        for b in bars:
            h = b.get_height()
            ax.text(b.get_x()+b.get_width()/2, h*1.01, f"{h:.2f}",
                    ha="center", va="bottom", fontsize=7)
    for j in range(idx+1, len(axes)):
        fig.delaxes(axes[j])
    fig.suptitle("Average Total GC Pause Time by Threads & Size", fontsize=16)
    fig.tight_layout(rect=[0, 0.03, 1, 0.95])
    out = os.path.join(OUT_DIR, "gc_pause_by_size.svg")
    fig.savefig(out, format="svg")
    plt.close(fig)
    print(f"[gc] wrote {out}")

def plot_gc_activity_heatmap(pause_agg, out_dir):
    """Plot a heatmap showing total GC pause time per (threads, size)."""
    if pause_agg.empty:
        print("[gc] No GC activity data to plot.")
        return
    pivot = pause_agg.pivot(index="threads", columns="size", values="total_pause_s")
    if pivot.empty:
        print("[gc] No data to generate heatmap.")
        return
    plt.figure(figsize=(12, 6))
    sns.heatmap(pivot, annot=True, fmt=".2f", cmap="YlOrRd", linewidths=0.5)
    plt.title("GC Pause Time (s) by Threads and Size")
    plt.xlabel("Input Size")
    plt.ylabel("Thread Count")
    plt.tight_layout()
    out_path = os.path.join(out_dir, "gc_activity_heatmap.svg")
    plt.savefig(out_path, format="svg")
    plt.close()
    print(f"[gc] wrote {out_path}")

def main():
    df = load_and_tag_gc()
    if df.empty:
        print("No GC data found in", GC_DIR)
        return
    pause_agg = aggregate_pause(df)
    plot_pause_grid(pause_agg)
    plot_gc_activity_heatmap(pause_agg, OUT_DIR)

if __name__ == "__main__":
    main()

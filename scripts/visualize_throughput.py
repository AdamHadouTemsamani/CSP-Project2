#!/usr/bin/env python3
import re
import pandas as pd
import matplotlib.pyplot as plt
import glob
import os

HEADER_RE = re.compile(r"-- threads=(\d+)\s+size=(\d+)\s+--")
METRIC_RE = re.compile(r"([\d,]+)\s+(\S+)")

def parse_perf_file(path):
    """
    Returns DataFrame with columns: threads, size, metric, value
    """
    records = []
    current = {'threads': None, 'size': None}
    in_block = False

    with open(path) as f:
        for raw in f:
            line = raw.strip()
            if not line:
                continue

            m = HEADER_RE.match(line)
            if m:
                current['threads'] = int(m.group(1))
                current['size']    = int(m.group(2))
                in_block = False
                continue

            if line.startswith("Performance counter stats"):
                in_block = True
                continue

            if in_block:
                if "seconds time elapsed" in line:
                    in_block = False
                    continue
                m2 = METRIC_RE.search(line)
                if m2:
                    val    = float(m2.group(1).replace(",", ""))
                    metric = m2.group(2)
                    records.append({
                        'threads': current['threads'],
                        'size':    current['size'],
                        'metric':  metric,
                        'value':   val
                    })

    return pd.DataFrame(records)

def plot_metric(df, metric, label, outpath):
    plt.figure()
    for t in sorted(df['threads'].unique()):
        sub = df[(df['threads']==t) & (df['metric']==metric)]
        mean = sub.groupby('size')['value'].mean().reset_index()
        plt.plot(mean['size'], mean['value'], marker='o', label=f"T={t}")
    plt.xscale('log')
    plt.xlabel("Array Size")
    plt.ylabel(metric)
    plt.title(f"{label} – {metric}")
    plt.grid(True)
    plt.legend(title="Threads", fontsize='small', loc='best')
    plt.tight_layout()
    plt.savefig(outpath, bbox_inches='tight')
    plt.close()

def main():
    perf_dir   = "perf"
    output_dir = os.path.join("images", "perf")
    os.makedirs(output_dir, exist_ok=True)

    for txt_path in glob.glob(os.path.join(perf_dir, "perf_*.txt")):
        label = os.path.basename(txt_path).replace("perf_", "").replace(".txt", "")
        df    = parse_perf_file(txt_path)
        if df.empty:
            print(f"[!] no data in {txt_path}")
            continue

        for metric in sorted(df['metric'].unique()):
            out = os.path.join(output_dir, f"{label}_{metric}.svg")
            print(f"Plotting {label} / {metric} → {out}")
            plot_metric(df, metric, label, out)

if __name__ == "__main__":
    main()

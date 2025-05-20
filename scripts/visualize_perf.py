#!/usr/bin/env python3
import re
import pandas as pd
import matplotlib.pyplot as plt
import glob
import os
import math

HEADER_RE = re.compile(r"-- threads=(\d+) size=(\d+) --")
METRIC_RE = re.compile(r"([\d,]+)\s+(\S+)")

def parse_perf_file(path):
    """
    Returns a DataFrame with columns: threads, size, metric, value
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


def plot_all_metrics(df, lang, output_file):
    """
    Given a perf-DataFrame for one language, plot every metric
    in a grid of subplots, one line per thread count.
    """
    metrics = sorted(df['metric'].unique())
    nmetrics = len(metrics)
    ncols = 2
    nrows = math.ceil(nmetrics / ncols)

    fig, axes = plt.subplots(nrows=nrows, ncols=ncols,
                             figsize=(ncols*5, nrows*4))
    # flatten axes
    axes_list = axes.flatten() if nrows*ncols > 1 else [axes]

    for idx, metric in enumerate(metrics):
        ax = axes_list[idx]
        sub = df[df['metric'] == metric]
        for t in sorted(sub['threads'].unique()):
            tdf = sub[sub['threads'] == t]
            mean = tdf.groupby('size')['value'].mean().reset_index()
            ax.plot(mean['size'], mean['value'], marker='o', label=f"T={t}")
        ax.set_xscale('log')
        ax.set_title(metric)
        ax.set_xlabel("Array size")
        ax.set_ylabel("Count")
        ax.grid(True)
        ax.legend(title="Threads", fontsize='small', loc='best')

    # remove unused axes
    for j in range(idx+1, len(axes_list)):
        fig.delaxes(axes_list[j])

    fig.suptitle(f"Perf Counters – {lang.upper()}", fontsize=16)
    fig.tight_layout(rect=[0,0.03,1,0.95])
    plt.savefig(output_file, bbox_inches='tight')
    plt.close()


def main():
    perf_dir   = "perf"
    output_dir = os.path.join("images", "perf")
    os.makedirs(output_dir, exist_ok=True)

    for txt in glob.glob(os.path.join(perf_dir, "perf_*.txt")):
        lang = os.path.basename(txt).replace("perf_", "").replace(".txt", "")
        df   = parse_perf_file(txt)
        if df.empty:
            print(f"[!] no data in {txt}")
            continue
        out_svg = os.path.join(output_dir, f"{lang}_perf.svg")
        print(f"Plotting all perf metrics for {lang} → {out_svg}")
        plot_all_metrics(df, lang, out_svg)


if __name__ == "__main__":
    main()

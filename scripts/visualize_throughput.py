#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import glob
import os

def load_throughput_file(path):
    # expects columns: phase,threads,size,seconds,mips
    return pd.read_csv(path)

def plot_throughput(df, lang, outpath):
    plt.figure()
    for t in sorted(df['threads'].unique()):
        sub = df[df['threads']==t]
        mean = sub.groupby('size')['mips'].mean().reset_index()
        plt.plot(mean['size'], mean['mips'], marker='o', label=f"Threads={t}")
    plt.xscale('log')
    plt.xlabel("Array Size")
    plt.ylabel("MIPS")
    plt.title(f"Throughput – {lang.upper()}")
    plt.grid(True)
    plt.legend(title="Threads", fontsize='small', loc='best')
    plt.tight_layout()
    plt.savefig(outpath, bbox_inches='tight')
    plt.close()

def main():
    results_dir = "results"
    output_dir  = os.path.join("images", "throughput")
    os.makedirs(output_dir, exist_ok=True)

    for csv_path in glob.glob(os.path.join(results_dir, "throughput_*.csv")):
        # e.g. throughput_c.csv → lang = c
        lang = os.path.basename(csv_path).split("_")[1].split(".")[0]
        df   = load_throughput_file(csv_path)
        out  = os.path.join(output_dir, f"{lang}_throughput.svg")
        print(f"Plotting {lang} → {out}")
        plot_throughput(df, lang, out)

if __name__ == "__main__":
    main()

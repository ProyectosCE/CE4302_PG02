#!/usr/bin/env python3

import re
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt


# PATHS
SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_DIR = SCRIPT_DIR.parent

RESULTS_DIR = PROJECT_DIR / "results"
PLOTS_DIR = RESULTS_DIR / "plots"

PLOTS_DIR.mkdir(parents=True, exist_ok=True)


# STYLE
plt.style.use("dark_background")

TITLE_SIZE = 16
LABEL_SIZE = 12

DATASETS = ["small", "medium", "large"]


# HELPERS
def extract_execution_time(perf_file):
    text = perf_file.read_text()

    match = re.search(
        r'([\d,]+)\s+\+\-\s+[\d,]+\s+seconds time elapsed',
        text
    )

    if not match:
        raise RuntimeError(f"Could not parse execution time from {perf_file}")

    return float(match.group(1).replace(",", ".")) * 1000.0


def extract_ipc(perf_file):
    text = perf_file.read_text()

    match = re.search(
        r'#\s+([\d,]+)\s+insn per cycle',
        text
    )

    if not match:
        raise RuntimeError(f"Could not parse IPC from {perf_file}")

    return float(match.group(1).replace(",", "."))


def extract_gpu_summary(summary_file):
    text = summary_file.read_text()

    def grab(pattern):
        m = re.search(pattern, text)
        if not m:
            raise RuntimeError(f"Missing field in {summary_file}")
        return float(m.group(1))

    return {
        "h2d_signal": grab(r'Average H2D signal transfer\s*:\s*([\d\.]+)'),
        "h2d_filters": grab(r'Average H2D filters transfer\s*:\s*([\d\.]+)'),
        "kernel": grab(r'Average kernel execution\s*:\s*([\d\.]+)'),
        "d2h": grab(r'Average D2H output transfer\s*:\s*([\d\.]+)'),
        "pipeline": grab(r'Average total GPU pipeline\s*:\s*([\d\.]+)')
    }


def add_labels(ax):
    for container in ax.containers:
        ax.bar_label(
            container,
            fmt="%.2f",
            padding=3,
            fontsize=9
        )


# LOAD DATA
scalar_times = []
simd_times = []
gpu_times = []

scalar_ipc = []
simd_ipc = []

gpu_profiles = {}

for dataset in DATASETS:

    scalar_perf = RESULTS_DIR / "scalar" / "perf" / f"{dataset}_perf.txt"
    simd_perf = RESULTS_DIR / "simd" / "perf" / f"{dataset}_perf.txt"

    gpu_summary = (
        RESULTS_DIR
        / "gpu"
        / "profiling"
        / f"{dataset}_gpu_summary.txt"
    )

    scalar_times.append(extract_execution_time(scalar_perf))
    simd_times.append(extract_execution_time(simd_perf))

    scalar_ipc.append(extract_ipc(scalar_perf))
    simd_ipc.append(extract_ipc(simd_perf))

    gpu_profiles[dataset] = extract_gpu_summary(gpu_summary)

    gpu_times.append(gpu_profiles[dataset]["pipeline"])


# FIGURE 1
# EXECUTION TIME
x = np.arange(len(DATASETS))
width = 0.25

fig, ax = plt.subplots(figsize=(10, 6))

b1 = ax.bar(
    x - width,
    scalar_times,
    width,
    label="Scalar",
    color="#1f77b4"
)

b2 = ax.bar(
    x,
    simd_times,
    width,
    label="SIMD",
    color="#ff7f0e"
)

b3 = ax.bar(
    x + width,
    gpu_times,
    width,
    label="GPU",
    color="#2ca02c"
)

ax.grid(
    axis="y",
    linestyle="--",
    alpha=0.3
)

ax.set_title("Execution Time by Dataset", fontsize=TITLE_SIZE)
ax.set_xlabel("Dataset", fontsize=LABEL_SIZE)
ax.set_ylabel("Time (ms)", fontsize=LABEL_SIZE)

ax.set_xticks(x)
ax.set_xticklabels(["Small", "Medium", "Large"])

ax.legend()

add_labels(ax)

plt.tight_layout()

plt.savefig(
    PLOTS_DIR / "execution_time.svg",
    format="svg",
    bbox_inches="tight"
)

plt.close()


# FIGURE 2
# SPEEDUP
speedup_simd = [
    scalar_times[i] / simd_times[i]
    for i in range(3)
]

speedup_gpu = [
    scalar_times[i] / gpu_times[i]
    for i in range(3)
]

fig, ax = plt.subplots(figsize=(10, 6))

b1 = ax.bar(
    x - width / 2,
    speedup_simd,
    width,
    label="SIMD",
    color="#ff7f0e"
)

b2 = ax.bar(
    x + width / 2,
    speedup_gpu,
    width,
    label="GPU",
    color="#2ca02c"
)

ax.grid(
    axis="y",
    linestyle="--",
    alpha=0.3
)

ax.set_title("Speedup Relative to Scalar", fontsize=TITLE_SIZE)

ax.set_xlabel("Dataset", fontsize=LABEL_SIZE)
ax.set_ylabel("Speedup (x)", fontsize=LABEL_SIZE)

ax.set_xticks(x)
ax.set_xticklabels(["Small", "Medium", "Large"])

ax.legend()

add_labels(ax)

plt.tight_layout()

plt.savefig(
    PLOTS_DIR / "speedup.svg",
    format="svg",
    bbox_inches="tight"
)

plt.close()


# FIGURE 3A / 3B / 3C
# GPU PIPELINE PER DATASET
for dataset in DATASETS:

    profile = gpu_profiles[dataset]

    labels = [
        "H2D Signal",
        "H2D Filters",
        "Kernel",
        "D2H Output"
    ]

    values = [
        profile["h2d_signal"],
        profile["h2d_filters"],
        profile["kernel"],
        profile["d2h"]
    ]

    fig, ax = plt.subplots(figsize=(10, 6))

    bars = ax.bar(
        labels,
        values,
        color=[
            "#4e79a7",
            "#f28e2b",
            "#e15759",
            "#76b7b2"
        ]
    )

    ax.set_title(
        f"GPU Pipeline Breakdown ({dataset.capitalize()} Dataset)",
        fontsize=TITLE_SIZE
    )

    ax.set_ylabel("Time (ms)")

    ax.set_yscale("log")

    ax.grid(
        axis="y",
        linestyle="--",
        alpha=0.3
    )

    for bar, value in zip(bars, values):
        ax.text(
            bar.get_x() + bar.get_width()/2,
            value,
            f"{value:.3f}",
            ha="center",
            va="bottom",
            fontsize=9
        )

    plt.tight_layout()

    plt.savefig(
        PLOTS_DIR / f"gpu_pipeline_{dataset}.svg",
        format="svg",
        bbox_inches="tight"
    )

    plt.close()


# FIGURE 4
# IPC
fig, ax = plt.subplots(figsize=(10, 6))

b1 = ax.bar(
    x - width / 2,
    scalar_ipc,
    width,
    label="Scalar",
    color="#1f77b4"
)

b2 = ax.bar(
    x + width / 2,
    simd_ipc,
    width,
    label="SIMD",
    color="#ff7f0e"
)

ax.grid(
    axis="y",
    linestyle="--",
    alpha=0.3
)

ax.set_title("Instructions Per Cycle (IPC)", fontsize=TITLE_SIZE)

ax.set_xlabel("Dataset", fontsize=LABEL_SIZE)
ax.set_ylabel("IPC", fontsize=LABEL_SIZE)

ax.set_xticks(x)
ax.set_xticklabels(["Small", "Medium", "Large"])

ax.legend()

add_labels(ax)

plt.tight_layout()

plt.savefig(
    PLOTS_DIR / "ipc.svg",
    format="svg",
    bbox_inches="tight"
)

plt.close()


print()
print("Generated figures:")
print(f"  {PLOTS_DIR / 'execution_time.svg'}")
print(f"  {PLOTS_DIR / 'speedup.svg'}")
print(f"  {PLOTS_DIR / 'gpu_pipeline_small.svg'}")
print(f"  {PLOTS_DIR / 'gpu_pipeline_medium.svg'}")
print(f"  {PLOTS_DIR / 'gpu_pipeline_large.svg'}")
print(f"  {PLOTS_DIR / 'ipc.svg'}")
print()
print("All figures generated successfully.")
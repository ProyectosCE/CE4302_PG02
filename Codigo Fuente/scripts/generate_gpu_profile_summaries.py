import re
from pathlib import Path


# ============================================================
# Paths
# ============================================================

SCRIPT_DIR = Path(__file__).resolve().parent

PROJECT_ROOT = SCRIPT_DIR.parent

PROFILING_DIR = PROJECT_ROOT / "results" / "gpu" / "profiling"

INPUT_FILE = PROFILING_DIR / "gpu_profiling.txt"


# ============================================================
# Dataset classification
# ============================================================

def classify_dataset(signal_size, filter_order):
    """
    Clasifica automáticamente el dataset según dimensiones.
    Ajusta estos valores si tu proyecto usa otros tamaños.
    """

    if signal_size <= 100000 and filter_order <= 128:
        return "small"

    if signal_size <= 1000000 and filter_order <= 512:
        return "medium"

    return "large"


# ============================================================
# Parsing helpers
# ============================================================

def extract_float(pattern, text):
    match = re.search(pattern, text)

    if not match:
        return None

    return float(match.group(1))


def extract_int(pattern, text):
    match = re.search(pattern, text)

    if not match:
        return None

    return int(match.group(1))


# ============================================================
# Parse profiling file
# ============================================================

if not INPUT_FILE.exists():
    print(f"Error: profiling file not found: {INPUT_FILE}")
    exit(1)


with open(INPUT_FILE, "r") as file:
    content = file.read()


blocks = content.split("=========== GPU PROFILING ===========")

datasets = {
    "small": [],
    "medium": [],
    "large": []
}


for block in blocks:

    if "Signal size" not in block:
        continue

    signal_size = extract_int(
        r"Signal size\s*:\s*(\d+)",
        block
    )

    filter_order = extract_int(
        r"Filter order\s*:\s*(\d+)",
        block
    )

    filter_count = extract_int(
        r"Filter count\s*:\s*(\d+)",
        block
    )

    h2d_signal = extract_float(
        r"H2D signal transfer\s*:\s*([\d.]+)",
        block
    )

    h2d_filters = extract_float(
        r"H2D filters transfer\s*:\s*([\d.]+)",
        block
    )

    kernel_execution = extract_float(
        r"Kernel execution\s*:\s*([\d.]+)",
        block
    )

    d2h_output = extract_float(
        r"D2H output transfer\s*:\s*([\d.]+)",
        block
    )

    total_pipeline = extract_float(
        r"Total GPU pipeline\s*:\s*([\d.]+)",
        block
    )

    dataset_name = classify_dataset(
        signal_size,
        filter_order
    )

    datasets[dataset_name].append({
        "signal_size": signal_size,
        "filter_order": filter_order,
        "filter_count": filter_count,
        "h2d_signal": h2d_signal,
        "h2d_filters": h2d_filters,
        "kernel_execution": kernel_execution,
        "d2h_output": d2h_output,
        "total_pipeline": total_pipeline
    })


# ============================================================
# Average computation
# ============================================================

def compute_average(entries, key):

    if not entries:
        return 0.0

    total = sum(entry[key] for entry in entries)

    return total / len(entries)


# ============================================================
# Generate summary files
# ============================================================

for dataset_name, entries in datasets.items():

    output_file = (
        PROFILING_DIR /
        f"{dataset_name}_gpu_summary.txt"
    )

    with open(output_file, "w") as file:

        file.write(
            "=========== GPU PROFILING SUMMARY ===========\n"
        )

        file.write(
            f"Dataset              : {dataset_name}\n"
        )

        file.write(
            f"Executions analyzed  : {len(entries)}\n\n"
        )

        if len(entries) == 0:

            file.write("No profiling data found.\n")

        else:

            avg_signal_size = compute_average(
                entries,
                "signal_size"
            )

            avg_filter_order = compute_average(
                entries,
                "filter_order"
            )

            avg_filter_count = compute_average(
                entries,
                "filter_count"
            )

            avg_h2d_signal = compute_average(
                entries,
                "h2d_signal"
            )

            avg_h2d_filters = compute_average(
                entries,
                "h2d_filters"
            )

            avg_kernel = compute_average(
                entries,
                "kernel_execution"
            )

            avg_d2h = compute_average(
                entries,
                "d2h_output"
            )

            avg_total = compute_average(
                entries,
                "total_pipeline"
            )

            file.write(
                f"Average signal size         : {avg_signal_size:.0f}\n"
            )

            file.write(
                f"Average filter order        : {avg_filter_order:.0f}\n"
            )

            file.write(
                f"Average filter count        : {avg_filter_count:.0f}\n\n"
            )

            file.write(
                f"Average H2D signal transfer : {avg_h2d_signal:.3f} ms\n"
            )

            file.write(
                f"Average H2D filters transfer: {avg_h2d_filters:.3f} ms\n"
            )

            file.write(
                f"Average kernel execution    : {avg_kernel:.3f} ms\n"
            )

            file.write(
                f"Average D2H output transfer : {avg_d2h:.3f} ms\n"
            )

            file.write(
                f"Average total GPU pipeline  : {avg_total:.3f} ms\n"
            )

        file.write(
            "\n=============================================\n"
        )

    print(f"Generated: {output_file}")


print("\nGPU profiling summaries generated successfully.")

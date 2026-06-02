#!/usr/bin/env python3

"""
Script para generar todas las figuras y gráficos del proyecto a partir de los resultados medidos.

Este módulo lee archivos de perfilado y resumen de ejecución para construir figuras comparativas entre las
implementaciones scalar, SIMD y GPU. Las salidas se guardan en formato SVG dentro del directorio de plots.

Uso:
    Ejecutar directamente para generar todas las figuras del análisis de rendimiento.

Autor: [Tu Nombre]
"""

import re
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt


# =====================
# RUTAS DEL PROYECTO
# =====================
SCRIPT_DIR = Path(__file__).resolve().parent  # Directorio donde reside este script
PROJECT_DIR = SCRIPT_DIR.parent  # Raíz del proyecto de Código Fuente

RESULTS_DIR = PROJECT_DIR / "results"  # Directorio base de resultados
PLOTS_DIR = RESULTS_DIR / "plots"  # Directorio de salida para las figuras

PLOTS_DIR.mkdir(parents=True, exist_ok=True)  # Crea la carpeta de plots si no existe


# =====================
# ESTILO DE GRÁFICAS
# =====================
plt.style.use("dark_background")  # Tema visual de alto contraste

TITLE_SIZE = 16  # Tamaño de fuente para títulos
LABEL_SIZE = 12  # Tamaño de fuente para etiquetas de ejes

DATASETS = ["small", "medium", "large"]  # Conjunto de datasets a procesar


# =====================
# FUNCIONES AUXILIARES
# =====================
def extract_execution_time(perf_file):
    """
    Extrae el tiempo de ejecución en milisegundos desde un archivo de perfilado.

    :param perf_file: Archivo de texto con la salida de perfilado.
    :type perf_file: Path
    :return: Tiempo de ejecución en milisegundos.
    :rtype: float
    """
    text = perf_file.read_text()  # Lee todo el contenido del archivo

    match = re.search(
        r'([\d,]+)\s+\+\-\s+[\d,]+\s+seconds time elapsed',
        text
    )  # Busca la línea con el tiempo reportado por perf

    if not match:
        raise RuntimeError(f"Could not parse execution time from {perf_file}")

    return float(match.group(1).replace(",", ".")) * 1000.0  # Convierte de segundos a ms


def extract_ipc(perf_file):
    """
    Extrae el valor de IPC desde un archivo de perfilado.

    :param perf_file: Archivo de texto con la salida de perfilado.
    :type perf_file: Path
    :return: Valor de IPC.
    :rtype: float
    """
    text = perf_file.read_text()  # Lee todo el contenido del archivo

    match = re.search(
        r'#\s+([\d,]+)\s+insn per cycle',
        text
    )  # Busca la métrica de instrucciones por ciclo

    if not match:
        raise RuntimeError(f"Could not parse IPC from {perf_file}")

    return float(match.group(1).replace(",", "."))


def extract_gpu_summary(summary_file):
    """
    Extrae el resumen de perfilado de GPU desde un archivo de texto.

    :param summary_file: Archivo de resumen de perfilado GPU.
    :type summary_file: Path
    :return: Diccionario con tiempos promedio de cada etapa del pipeline.
    :rtype: dict
    """
    text = summary_file.read_text()  # Lee todo el contenido del archivo

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
    """
    Agrega etiquetas numéricas a las barras de un gráfico.

    :param ax: Eje de Matplotlib que contiene las barras.
    :type ax: matplotlib.axes.Axes
    :return: None
    """
    for container in ax.containers:
        ax.bar_label(
            container,
            fmt="%.2f",
            padding=3,
            fontsize=9
        )  # Etiqueta cada barra con dos decimales


# =====================
# CARGA DE DATOS
# =====================
scalar_times = []  # Tiempos de ejecución de la implementación scalar
simd_times = []  # Tiempos de ejecución de la implementación SIMD
gpu_times = []  # Tiempos totales del pipeline GPU

scalar_ipc = []  # IPC de la implementación scalar
simd_ipc = []  # IPC de la implementación SIMD

gpu_profiles = {}  # Resúmenes de perfilado GPU por dataset

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

    gpu_times.append(gpu_profiles[dataset]["pipeline"])  # Tiempo total del pipeline GPU


# =====================
# FIGURA 1: TIEMPO DE EJECUCIÓN
# =====================
x = np.arange(len(DATASETS))  # Posiciones base para las barras
width = 0.25  # Ancho de cada barra

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
)  # Cuadrícula horizontal tenue

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


# =====================
# FIGURA 2: SPEEDUP
# =====================
speedup_simd = [
    scalar_times[i] / simd_times[i]
    for i in range(3)
]  # Speedup de SIMD respecto a scalar

speedup_gpu = [
    scalar_times[i] / gpu_times[i]
    for i in range(3)
]  # Speedup de GPU respecto a scalar

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


# =====================
# FIGURAS 3A / 3B / 3C: PIPELINE GPU
# =====================
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

    ax.set_yscale("log")  # Escala logarítmica para visualizar etapas de distinta magnitud

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
        )  # Anota el valor exacto sobre cada barra

    plt.tight_layout()

    plt.savefig(
        PLOTS_DIR / f"gpu_pipeline_{dataset}.svg",
        format="svg",
        bbox_inches="tight"
    )

    plt.close()


# =====================
# FIGURA 4: IPC
# =====================
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


# =====================
# MENSAJE FINAL
# =====================
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
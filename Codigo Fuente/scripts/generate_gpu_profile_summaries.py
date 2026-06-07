"""
Script para generar resúmenes consolidados del perfilado de GPU.

Este script analiza el archivo de perfilado bruto, clasifica cada ejecución según el tamaño del dataset,
agrupa las métricas por categoría y genera archivos de resumen con promedios por dataset.

Uso:
    Ejecutar directamente para producir los archivos *_gpu_summary.txt dentro del directorio de profiling.

Autor: [Tu Nombre]
"""

import re
from pathlib import Path


# ============================================================
# RUTAS
# ============================================================

SCRIPT_DIR = Path(__file__).resolve().parent  # Directorio donde reside este script

PROJECT_ROOT = SCRIPT_DIR.parent  # Raíz del proyecto

PROFILING_DIR = PROJECT_ROOT / "results" / "gpu" / "profiling"  # Carpeta de perfilado GPU

INPUT_FILE = PROFILING_DIR / "gpu_profiling.txt"  # Archivo de entrada con el perfilado bruto


# ============================================================
# CLASIFICACIÓN DEL DATASET
# ============================================================

def classify_dataset(signal_size, filter_order):
    """
    Clasifica automáticamente un dataset según el tamaño de la señal y el orden del filtro.

    :param signal_size: Número de muestras de la señal.
    :type signal_size: int
    :param filter_order: Orden del filtro FIR.
    :type filter_order: int
    :return: Nombre lógico del dataset: small, medium o large.
    :rtype: str
    """

    if signal_size <= 100000 and filter_order <= 128:
        return "small"

    if signal_size <= 1000000 and filter_order <= 512:
        return "medium"

    return "large"


# ============================================================
# HELPERS DE PARSEO
# ============================================================

def extract_float(pattern, text):
    """
    Extrae un valor flotante desde un bloque de texto usando una expresión regular.

    :param pattern: Patrón regular con un grupo de captura numérico.
    :type pattern: str
    :param text: Texto donde se buscará el valor.
    :type text: str
    :return: Valor flotante si se encuentra; en caso contrario, None.
    :rtype: float | None
    """
    match = re.search(pattern, text)

    if not match:
        return None

    return float(match.group(1))


def extract_int(pattern, text):
    """
    Extrae un valor entero desde un bloque de texto usando una expresión regular.

    :param pattern: Patrón regular con un grupo de captura numérico.
    :type pattern: str
    :param text: Texto donde se buscará el valor.
    :type text: str
    :return: Valor entero si se encuentra; en caso contrario, None.
    :rtype: int | None
    """
    match = re.search(pattern, text)

    if not match:
        return None

    return int(match.group(1))


# ============================================================
# PARSEO DEL ARCHIVO DE PERFILADO
# ============================================================

if not INPUT_FILE.exists():
    print(f"Error: profiling file not found: {INPUT_FILE}")
    exit(1)


with open(INPUT_FILE, "r") as file:
    content = file.read()  # Lee el contenido completo del archivo de perfilado


blocks = content.split("=========== GPU PROFILING ===========")  # Divide el archivo en bloques de ejecución

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
    )  # Clasificación del bloque por tamaño de dataset

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
# CÁLCULO DE PROMEDIOS
# ============================================================

def compute_average(entries, key):
    """
    Calcula el promedio de una métrica específica dentro de una lista de entradas.

    :param entries: Lista de diccionarios con métricas de perfilado.
    :type entries: list[dict]
    :param key: Clave de la métrica a promediar.
    :type key: str
    :return: Promedio de la métrica o 0.0 si no hay entradas.
    :rtype: float
    """

    if not entries:
        return 0.0

    total = sum(entry[key] for entry in entries)  # Suma acumulada de la métrica seleccionada

    return total / len(entries)


# ============================================================
# GENERACIÓN DE RESÚMENES
# ============================================================

for dataset_name, entries in datasets.items():

    output_file = (
        PROFILING_DIR /
        f"{dataset_name}_gpu_summary.txt"
    )  # Archivo de salida por dataset

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

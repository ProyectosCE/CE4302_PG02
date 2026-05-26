"""
Script para validar los tamaños de los archivos de salida generados por las implementaciones FIR.

Este script recorre los directorios de resultados de cada implementación y dataset,
verificando que cada archivo binario de salida tenga el tamaño esperado según la cantidad de muestras y filtros.

Uso:
    Ejecutar directamente para imprimir el estado de validación de los archivos de salida.
"""

from pathlib import Path

# CONFIGURACIÓN GLOBAL
DATASETS = {
    "small": 100000,      # Número de muestras para el dataset pequeño
    "medium": 1000000,    # Número de muestras para el dataset mediano
    "large": 10000000     # Número de muestras para el dataset grande
}

NUM_FILTERS = 16  # Número de filtros FIR por dataset

IMPLEMENTATIONS = [
    "scalar",  # Implementación secuencial
    "simd",    # Implementación vectorizada SIMD
    "gpu"      # Implementación en GPU
]

FLOAT_SIZE_BYTES = 4  # Tamaño de un float en bytes

# RUTAS DEL PROYECTO
SCRIPT_DIR = Path(__file__).resolve().parent  # Directorio donde está este script

PROJECT_ROOT = SCRIPT_DIR.parent  # Raíz del proyecto

RESULTS_DIR = PROJECT_ROOT / "results"  # Carpeta de resultados

# VALIDACIÓN DE ARCHIVOS
print("")
print("=========================================")
print("VALIDATING OUTPUT FILE SIZES")
print("=========================================")

for implementation in IMPLEMENTATIONS:
    output_dir = (
        RESULTS_DIR /
        implementation /
        "output_signal"
    )

    print("")
    print(f"[IMPLEMENTATION] {implementation}")

    # Verifica si existe el directorio de salida para la implementación
    if not output_dir.exists():
        print(f"  Output directory not found: {output_dir}")
        print("  Skipping...")
        continue

    for dataset_name, signal_size in DATASETS.items():
        expected_size = signal_size * FLOAT_SIZE_BYTES  # Tamaño esperado del archivo

        for filter_id in range(NUM_FILTERS):
            filename = (
                f"{dataset_name}_filter_{filter_id:02d}.bin"
            )  # Nombre del archivo de salida

            filepath = output_dir / filename  # Ruta completa al archivo

            # Verifica si el archivo existe
            if not filepath.exists():
                print(f"  Missing file: {filename}")
                continue

            actual_size = filepath.stat().st_size  # Tamaño real del archivo

            # Compara el tamaño real con el esperado
            if actual_size == expected_size:
                print(
                    f"  [OK] {filename} "
                    f"({actual_size} bytes)"
                )
            else:
                print(
                    f"  [ERROR] {filename} "
                    f"(expected={expected_size}, "
                    f"actual={actual_size})"
                )

print("")
print("Validation completed.")
print("")
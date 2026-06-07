"""
Script para comparar los archivos de salida generados por diferentes implementaciones FIR.

Este script compara los archivos binarios de salida de las implementaciones 'simd' y 'gpu' contra la referencia 'scalar',
calculando el error absoluto máximo y medio para cada filtro y dataset.

Uso:
    Ejecutar directamente para imprimir el estado de comparación de los archivos de salida.
"""

from pathlib import Path
import numpy as np

# CONFIGURACIÓN GLOBAL
DATASETS = [
    "small",   # Dataset pequeño
    "medium",  # Dataset mediano
    "large"    # Dataset grande
]

NUM_FILTERS = 16  # Número de filtros FIR por dataset

REFERENCE_IMPLEMENTATION = "scalar"  # Implementación de referencia

COMPARISON_IMPLEMENTATIONS = [
    "simd",  # Implementación vectorizada SIMD
    "gpu"    # Implementación en GPU
]

# RUTAS DEL PROYECTO
SCRIPT_DIR = Path(__file__).resolve().parent  # Directorio donde está este script

PROJECT_ROOT = SCRIPT_DIR.parent  # Raíz del proyecto

RESULTS_DIR = PROJECT_ROOT / "results"  # Carpeta de resultados

# FUNCIONES AUXILIARES
def load_binary(filepath):
    """
    Carga un archivo binario de datos tipo float32 y lo retorna como un arreglo de numpy.

    :param filepath: Ruta al archivo binario.
    :type filepath: Path
    :return: Arreglo numpy con los datos cargados.
    :rtype: np.ndarray
    """
    return np.fromfile(
        filepath,
        dtype=np.float32
    )

# COMPARACIÓN DE SALIDAS
print("")
print("=========================================")
print("COMPARING OUTPUT SIGNALS")
print("=========================================")

reference_dir = (
    RESULTS_DIR /
    REFERENCE_IMPLEMENTATION /
    "output_signal"
)

# Verifica si existe el directorio de referencia
if not reference_dir.exists():
    print("")
    print("Reference scalar output directory not found.")
    print("Nothing to compare.")
    print("")
    exit(0)

for implementation in COMPARISON_IMPLEMENTATIONS:
    compare_dir = (
        RESULTS_DIR /
        implementation /
        "output_signal"
    )

    print("")
    print(f"[COMPARISON] scalar vs {implementation}")

    # Verifica si existe el directorio de la implementación a comparar
    if not compare_dir.exists():
        print(f"  Output directory not found: {compare_dir}")
        print("  Skipping...")
        continue

    for dataset_name in DATASETS:
        for filter_id in range(NUM_FILTERS):
            filename = (
                f"{dataset_name}_filter_{filter_id:02d}.bin"
            )  # Nombre del archivo de salida

            scalar_path = (
                reference_dir /
                filename
            )  # Ruta al archivo de referencia

            compare_path = (
                compare_dir /
                filename
            )  # Ruta al archivo a comparar

            # Verifica si existen ambos archivos
            if not scalar_path.exists():
                print(f"  Missing scalar file: {filename}")
                continue

            if not compare_path.exists():
                print(
                    f"  Missing comparison file: {filename}"
                )
                continue

            scalar_data = load_binary(scalar_path)  # Datos de referencia
            compare_data = load_binary(compare_path)  # Datos a comparar

            # Verifica que ambos archivos tengan el mismo tamaño
            if scalar_data.size != compare_data.size:
                print(
                    f"  [ERROR] Different sizes: {filename}"
                )
                continue

            abs_error = np.abs(
                scalar_data - compare_data
            )  # Error absoluto elemento a elemento

            max_error = np.max(abs_error)  # Error absoluto máximo
            mean_error = np.mean(abs_error)  # Error absoluto medio

            print(
                f"  [OK] {filename} | "
                f"max_error={max_error:.20f} | "
                f"mean_error={mean_error:.20f}"
            )

print("")
print("Comparison completed.")
print("")
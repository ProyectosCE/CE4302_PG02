"""
Script para validar la corrección numérica de las salidas generadas por la implementación escalar FIR.

Este script compara los archivos de salida generados por la implementación escalar con una referencia calculada en Python,
verificando que el error absoluto máximo y medio estén dentro de la tolerancia permitida.

Uso:
    Ejecutar directamente para imprimir el estado de validación de la corrección numérica.

Autor: [Tu Nombre]
"""

import os
import numpy as np

# CONFIGURACIÓN GLOBAL
DATASETS = {
    "small": {
        "signal_size": 100000,   # Número de muestras
        "filter_order": 128      # Orden del filtro FIR
    },
    "medium": {
        "signal_size": 1000000,
        "filter_order": 256
    },
    "large": {
        "signal_size": 10000000,
        "filter_order": 512
    }
}

NUM_FILTERS = 16  # Número de filtros FIR por dataset

BASE_DIR = os.path.dirname(
    os.path.dirname(
        os.path.abspath(__file__)
    )
)  # Raíz del proyecto

DATASETS_DIR = os.path.join(
    BASE_DIR,
    "datasets"
)  # Carpeta de datasets

RESULTS_DIR = os.path.join(
    BASE_DIR,
    "results",
    "scalar",
    "output_signal"
)  # Carpeta de resultados de la implementación escalar

ABS_TOLERANCE = 1e-4  # Tolerancia absoluta para la comparación numérica

# FUNCIONES AUXILIARES
def load_binary(filepath):
    """
    Carga un archivo binario de datos tipo float32 y lo retorna como un arreglo de numpy.

    :param filepath: Ruta al archivo binario.
    :type filepath: str
    :return: Arreglo numpy con los datos cargados.
    :rtype: np.ndarray
    """
    return np.fromfile(
        filepath,
        dtype=np.float32
    )

def fir_reference(signal, filt):
    """
    Calcula la salida de un filtro FIR de referencia en Python puro.

    :param signal: Señal de entrada.
    :type signal: np.ndarray
    :param filt: Coeficientes del filtro FIR.
    :type filt: np.ndarray
    :return: Salida filtrada.
    :rtype: np.ndarray
    """
    signal_size = signal.size
    filter_order = filt.size

    output = np.zeros(
        signal_size,
        dtype=np.float32
    )

    for n in range(signal_size):
        acc = 0.0  # Acumulador para la suma ponderada
        for k in range(filter_order):
            if n >= k:
                acc += (
                    filt[k] *
                    signal[n - k]
                )  # Multiplicación y acumulación
        output[n] = acc  # Almacena el resultado en la salida

    return output

# VALIDACIÓN DE CORRECCIÓN
print("")
print("====================================================")
print("VALIDATING SCALAR NUMERICAL CORRECTNESS")
print("====================================================")

# Verifica si existe el directorio de resultados
if not os.path.exists(RESULTS_DIR):
    print("")
    print("Scalar output directory not found.")
    print(f"Missing path: {RESULTS_DIR}")
    print("")
    exit(0)

for dataset_name, config in DATASETS.items():
    print("")
    print(f"[DATASET] {dataset_name}")

    dataset_dir = os.path.join(
        DATASETS_DIR,
        dataset_name
    )  # Carpeta del dataset actual

    signal_path = os.path.join(
        dataset_dir,
        "signal.bin"
    )  # Archivo de señal

    filters_path = os.path.join(
        dataset_dir,
        "filters.bin"
    )  # Archivo de filtros

    # Verifica existencia de archivos de entrada
    if not os.path.exists(signal_path):
        print(f"  Missing signal file: {signal_path}")
        continue

    if not os.path.exists(filters_path):
        print(f"  Missing filters file: {filters_path}")
        continue

    signal = load_binary(signal_path)  # Carga la señal
    filters = load_binary(filters_path)  # Carga los filtros

    filter_order = config["filter_order"]  # Orden del filtro FIR

    for filter_id in range(NUM_FILTERS):
        output_filename = (
            f"{dataset_name}_filter_{filter_id:02d}.bin"
        )  # Nombre del archivo de salida

        output_path = os.path.join(
            RESULTS_DIR,
            output_filename
        )  # Ruta al archivo de salida

        # Verifica existencia del archivo de salida
        if not os.path.exists(output_path):
            print(f"  Missing output file: {output_filename}")
            continue

        current_filter = filters[
            filter_id * filter_order:
            (filter_id + 1) * filter_order
        ]  # Extrae el filtro actual

        reference_output = fir_reference(
            signal,
            current_filter
        )  # Salida de referencia

        scalar_output = load_binary(
            output_path
        )  # Salida generada por la implementación escalar

        # Verifica que ambos archivos tengan el mismo tamaño
        if reference_output.size != scalar_output.size:
            print(
                f"  [ERROR] Different sizes: {output_filename}"
            )
            continue

        abs_error = np.abs(
            reference_output - scalar_output
        )  # Error absoluto elemento a elemento

        max_error = np.max(abs_error)  # Error absoluto máximo
        mean_error = np.mean(abs_error)  # Error absoluto medio

        # Verifica si la salida está dentro de la tolerancia
        if np.allclose(
            reference_output,
            scalar_output,
            atol=ABS_TOLERANCE
        ):
            print(
                f"  [OK] {output_filename} | "
                f"max_error={max_error:.8f} | "
                f"mean_error={mean_error:.8f}"
            )
        else:
            print(
                f"  [ERROR] {output_filename} | "
                f"max_error={max_error:.8f} | "
                f"mean_error={mean_error:.8f}"
            )

print("")
print("Correctness validation completed.")
print("")
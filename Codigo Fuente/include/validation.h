#ifndef VALIDATION_H
#define VALIDATION_H

#include <stddef.h>

/**
 * @file validation.h
 * @brief Funciones auxiliares para almacenamiento de resultados y validación.
 *
 * Este módulo permite almacenar los resultados generados por las implementaciones:
 *
 * - Scalar
 * - SIMD
 * - GPU
 *
 * Los resultados se almacenan en formato binario float32 para:
 *
 * - minimizar overhead de escritura,
 * - facilitar comparación entre implementaciones,
 * - permitir análisis posterior mediante Python,
 * - mantener compatibilidad con SIMD y GPU.
 */

/**
 * @brief Guarda una señal filtrada en formato binario float32.
 *
 * La función almacena un arreglo de floats utilizando escritura binaria directa.
 *
 * Formato:
 *
 * - float32 little-endian
 * - contiguous memory layout
 *
 * Este formato permite:
 *
 * - lectura rápida desde Python usando numpy.fromfile(),
 * - comparación eficiente entre implementaciones,
 * - evitar overhead de serialización.
 *
 * @param output_path Ruta completa del archivo binario de salida.
 * @param data Puntero al arreglo de datos a almacenar.
 * @param size Cantidad de elementos float a escribir.
 *
 * @return int
 * - 0 si la operación fue exitosa.
 * - -1 si ocurrió un error.
 */
int save_output_binary(
    const char* output_path,
    const float* data,
    size_t size
);

/**
 * @brief Construye automáticamente la ruta de salida para una implementación.
 *
 * Genera rutas con el siguiente formato:
 *
 * results/<implementation>/output_signal/<dataset>_filter_<id>.bin
 *
 * Ejemplo:
 *
 * results/scalar/output_signal/small_filter_03.bin
 *
 * @param output_path Buffer donde se almacenará la ruta generada.
 * @param max_length Tamaño máximo del buffer.
 * @param implementation Nombre de implementación:
 *        "scalar", "simd" o "gpu".
 * @param dataset_name Nombre del dataset:
 *        "small", "medium" o "large".
 * @param filter_id Identificador del filtro.
 */
void build_output_path(
    char* output_path,
    size_t max_length,
    const char* implementation,
    const char* dataset_name,
    int filter_id
);

#endif
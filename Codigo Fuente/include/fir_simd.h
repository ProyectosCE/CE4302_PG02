#ifndef FIR_SIMD_H
#define FIR_SIMD_H

#include "common.h"

/**
 * @file fir_simd.h
 * @brief Implementación SIMD AVX2/FMA del filtro FIR.
 */

/**
 * @brief Ejecuta un filtro FIR SIMD sobre una señal.
 *
 * Utiliza registros AVX2 de 256 bits para procesar
 * múltiples muestras simultáneamente.
 *
 * @param signal Señal de entrada.
 * @param filter Coeficientes del filtro FIR.
 * @param output Buffer de salida.
 * @param signal_size Tamaño de la señal.
 * @param filter_order Orden del filtro FIR.
 */
void fir_simd(
    const float* signal,
    const float* filter,
    float* output,
    size_t signal_size,
    size_t filter_order
);

/**
 * @brief Ejecuta benchmark completo de FIR SIMD.
 *
 * Ejecuta todos los filtros sobre la señal,
 * mide tiempo y guarda resultados binarios.
 *
 * @param dataset Dataset cargado.
 * @param dataset_name Nombre del dataset.
 * @param result Resultado benchmark.
 *
 * @return int
 * - 0 si fue exitoso.
 * - -1 si hubo error.
 */
int run_fir_simd_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
);

#endif
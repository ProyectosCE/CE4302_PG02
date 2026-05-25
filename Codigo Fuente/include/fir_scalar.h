#ifndef FIR_SCALAR_H
#define FIR_SCALAR_H

#include "common.h"

/**
 * @file fir_scalar.h
 * @brief Implementación secuencial del filtro FIR.
 */

/**
 * @brief Ejecuta un filtro FIR escalar sobre una señal.
 *
 * @param signal Señal de entrada.
 * @param filter Coeficientes del filtro FIR.
 * @param output Buffer de salida.
 * @param signal_size Tamaño de la señal.
 * @param filter_order Orden del filtro FIR.
 */
void fir_scalar(
    const float* signal,
    const float* filter,
    float* output,
    size_t signal_size,
    size_t filter_order
);

/**
 * @brief Ejecuta benchmark completo de FIR escalar.
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
int run_fir_scalar_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
);

#endif
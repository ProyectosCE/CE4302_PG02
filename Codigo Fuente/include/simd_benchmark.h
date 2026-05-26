#ifndef SIMD_BENCHMARK_H
#define SIMD_BENCHMARK_H

#include "common.h"
#include "dataset.h"

/**
 * @brief Ejecuta benchmark FIR SIMD.
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
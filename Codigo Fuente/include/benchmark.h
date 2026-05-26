#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "dataset.h"
#include "common.h"

/**
 * @file benchmark.h
 * @brief Interfaz común para benchmarks FIR.
 */

/**
 * @brief Imprime en consola los resultados de un benchmark FIR.
 *
 * @param result Resultado del benchmark.
 */
void print_benchmark_result(
    const benchmark_result_t* result
);

#if defined(BUILD_SCALAR)

/**
 * @brief Ejecuta el benchmark de la implementación escalar FIR.
 *
 * @param dataset Dataset cargado.
 * @param dataset_name Nombre del dataset.
 * @param result Resultado del benchmark.
 *
 * @return int 0 si fue exitoso, -1 si hubo error.
 */
int run_fir_scalar_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
);

#endif

#if defined(BUILD_SIMD)

/**
 * @brief Ejecuta el benchmark de la implementación SIMD FIR.
 *
 * @param dataset Dataset cargado.
 * @param dataset_name Nombre del dataset.
 * @param result Resultado del benchmark.
 *
 * @return int 0 si fue exitoso, -1 si hubo error.
 */
int run_fir_simd_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
);

#endif

#if defined(BUILD_GPU)

/**
 * @brief Ejecuta el benchmark de la implementación GPU FIR.
 *
 * @param dataset Dataset cargado.
 * @param dataset_name Nombre del dataset.
 * @param result Resultado del benchmark.
 *
 * @return int 0 si fue exitoso, -1 si hubo error.
 */
int run_fir_gpu_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
);

#endif

#endif
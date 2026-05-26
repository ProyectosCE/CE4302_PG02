#ifndef GPU_BENCHMARK_H
#define GPU_BENCHMARK_H

#include "dataset.h"
#include "common.h"

/**
 * @file gpu_benchmark.h
 * @brief Interfaz del benchmark FIR GPU.
 */

int run_fir_gpu_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
);

#endif
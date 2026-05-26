#include "../../include/common.h"

#include <stdio.h>

/**
 * @file benchmark_common.c
 * @brief Funciones comunes de benchmarks FIR.
 */

/**
 * @brief Imprime en consola los resultados de un benchmark FIR.
 *
 * Muestra el nombre del dataset, la implementación utilizada,
 * el tiempo de ejecución y el throughput.
 *
 * @param result Resultados del benchmark.
 */
void print_benchmark_result(
    const benchmark_result_t* result
)
{
    printf("\nBenchmark Result\n");
    printf("----------------\n");

    printf("Dataset        : %s\n",
           result->dataset_name);

    printf("Implementation : ");

    switch (result->implementation)
    {
        case IMPLEMENTATION_SCALAR:
            printf("SCALAR\n");
            break;

        case IMPLEMENTATION_SIMD:
            printf("SIMD\n");
            break;

        case IMPLEMENTATION_GPU:
            printf("GPU\n");
            break;

        default:
            printf("UNKNOWN\n");
            break;
    }

    printf("Execution Time : %.3f ms\n",
           result->execution_time_ms);

    printf("Throughput     : %.3f samples/sec\n",
           result->throughput);
}
#include "../../include/common.h"
#include "../../include/fir_scalar.h"

#include <stdio.h>

/**
 * @file benchmark.c
 * @brief Benchmark centralizado para implementaciones FIR.
 */

/**
 * @brief Imprime en consola los resultados de un benchmark FIR.
 *
 * Muestra el nombre del dataset, la implementación utilizada, el tiempo de ejecución y el rendimiento (throughput).
 *
 * @param result Puntero constante a la estructura benchmark_result_t con los resultados a mostrar.
 */
void print_benchmark_result(
    const benchmark_result_t* result /**< Resultados del benchmark a imprimir */
)
{
    printf("\nBenchmark Result\n");
    printf("----------------\n");

    printf("Dataset        : %s\n",
           result->dataset_name); /**< Nombre del dataset */

    printf("Implementation : ");

    switch (result->implementation)
    {
        case IMPLEMENTATION_SCALAR:
            printf("SCALAR\n"); /**< Implementación escalar */
            break;

        case IMPLEMENTATION_SIMD:
            printf("SIMD\n"); /**< Implementación SIMD */
            break;

        case IMPLEMENTATION_GPU:
            printf("GPU\n"); /**< Implementación GPU */
            break;

        default:
            printf("UNKNOWN\n"); /**< Implementación desconocida */
            break;
    }

    printf("Execution Time : %.3f ms\n",
           result->execution_time_ms); /**< Tiempo de ejecución en ms */

    printf("Throughput     : %.3f samples/sec\n",
           result->throughput); /**< Rendimiento en muestras por segundo */
}
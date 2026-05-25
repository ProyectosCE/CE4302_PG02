#include "../../include/common.h"

#include <stdio.h>

/**
 * @file benchmark.c
 * @brief Funciones para impresión de resultados de benchmarks de implementaciones FIR.
 */

/**
 * @brief Imprime en consola los resultados de un benchmark.
 *
 * Muestra el nombre del dataset, el tiempo de ejecución y el rendimiento (throughput) de la implementación evaluada.
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

    printf("Execution Time : %.3f ms\n",
           result->execution_time_ms); /**< Tiempo de ejecución en ms */

    printf("Throughput     : %.3f samples/sec\n",
           result->throughput); /**< Rendimiento en muestras por segundo */
}
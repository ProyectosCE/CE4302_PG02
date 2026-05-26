#include "../../include/scalar_benchmark.h"

#include "../../include/fir_scalar.h"
#include "../../include/aligned_memory.h"
#include "../../include/timer.h"
#include "../../include/validation.h"

#include <string.h>

/**
 * @file scalar_benchmark.c
 * @brief Benchmark para implementación FIR escalar.
 */

/**
 * @brief Ejecuta el benchmark de la implementación escalar FIR.
 *
 * Esta función centraliza:
 * - medición de tiempo,
 * - generación de outputs,
 * - cálculo de throughput,
 * - almacenamiento de resultados.
 *
 * @param dataset Dataset cargado.
 * @param dataset_name Nombre del dataset.
 * @param result Estructura de resultados.
 *
 * @return int
 * - 0 si fue exitoso.
 * - -1 si hubo error.
 */
int run_fir_scalar_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
)
{
    size_t output_size_bytes =
        dataset->signal_size * sizeof(float);

    float* output =
        (float*) aligned_malloc(output_size_bytes);

    if (!output)
    {
        return -1;
    }

    double start_time =
        get_time_ms();

    for (size_t filter_id = 0;
         filter_id < dataset->num_filters;
         filter_id++)
    {
        const float* current_filter =
            &dataset->filters[
                filter_id * dataset->filter_order
            ];

        fir_scalar(
            dataset->signal,
            current_filter,
            output,
            dataset->signal_size,
            dataset->filter_order
        );

        char output_path[MAX_PATH_LENGTH];

        build_output_path(
            output_path,
            MAX_PATH_LENGTH,
            "scalar",
            dataset_name,
            (int) filter_id
        );

        if (save_output_binary(
                output_path,
                output,
                dataset->signal_size) != 0)
        {
            aligned_free(output);

            return -1;
        }
    }

    double end_time =
        get_time_ms();

    result->execution_time_ms =
        end_time - start_time;

    double total_samples =
        (double) dataset->signal_size *
        (double) dataset->num_filters;

    result->throughput =
        total_samples /
        (result->execution_time_ms / 1000.0);

    strcpy(
        result->dataset_name,
        dataset_name
    );

    result->implementation =
        IMPLEMENTATION_SCALAR;

    aligned_free(output);

    return 0;
}
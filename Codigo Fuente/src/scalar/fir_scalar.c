#include "../../include/fir_scalar.h"
#include "../../include/aligned_memory.h"
#include "../../include/timer.h"
#include "../../include/validation.h"

#include <stdio.h>
#include <string.h>

/**
 * @file fir_scalar.c
 * @brief Implementación secuencial del filtro FIR.
 */

/**
 * @brief Ejecuta un filtro FIR escalar sobre una señal.
 *
 * Recorre la señal de entrada y aplica el filtro FIR de manera secuencial, acumulando el resultado en cada posición de salida.
 *
 * @param signal Señal de entrada.
 * @param filter Coeficientes del filtro FIR.
 * @param output Buffer de salida.
 * @param signal_size Tamaño de la señal.
 * @param filter_order Orden del filtro FIR.
 */
void fir_scalar(
    const float* signal,      /**< Señal de entrada */
    const float* filter,      /**< Coeficientes del filtro FIR */
    float* output,            /**< Buffer de salida */
    size_t signal_size,       /**< Tamaño de la señal */
    size_t filter_order       /**< Orden del filtro FIR */
)
{
    for (size_t n = 0; n < signal_size; n++)
    {
        float acc = 0.0f; /**< Acumulador para la suma ponderada */

        for (size_t k = 0; k < filter_order; k++)
        {
            if (n >= k)
            {
                acc += filter[k] * signal[n - k]; /**< Multiplicación y acumulación */
            }
        }

        output[n] = acc; /**< Almacena el resultado en la salida */
    }
}

/**
 * @brief Ejecuta el benchmark completo del filtro FIR escalar.
 *
 * Aplica todos los filtros FIR del dataset sobre la señal, mide el tiempo de ejecución,
 * calcula el rendimiento y guarda los resultados binarios de salida.
 *
 * @param dataset Dataset cargado con señal y filtros.
 * @param dataset_name Nombre del dataset.
 * @param result Estructura donde se almacenan los resultados del benchmark.
 * @return int 0 si fue exitoso, -1 si hubo error.
 */
int run_fir_scalar_benchmark(
    const dataset_t* dataset,        /**< Dataset cargado */
    const char* dataset_name,        /**< Nombre del dataset */
    benchmark_result_t* result       /**< Resultado del benchmark */
)
{
    size_t output_size_bytes =
        dataset->signal_size * sizeof(float); /**< Tamaño del buffer de salida */

    float* output =
        (float*) aligned_malloc(output_size_bytes); /**< Reserva de memoria alineada para la salida */

    if (!output)
    {
        return -1;
    }

    double start_time = get_time_ms(); /**< Marca de tiempo inicial */

    for (size_t filter_id = 0;
         filter_id < dataset->num_filters;
         filter_id++)
    {
        const float* current_filter =
            &dataset->filters[
                filter_id * dataset->filter_order
            ]; /**< Selección del filtro actual */

        fir_scalar(
            dataset->signal,
            current_filter,
            output,
            dataset->signal_size,
            dataset->filter_order
        ); /**< Aplicación del filtro FIR escalar */

        char output_path[MAX_PATH_LENGTH]; /**< Buffer para la ruta de salida */

        build_output_path(
            output_path,
            MAX_PATH_LENGTH,
            "scalar",
            dataset_name,
            (int) filter_id
        ); /**< Construcción de la ruta de salida */

        if (save_output_binary(
                output_path,
                output,
                dataset->signal_size) != 0)
        {
            aligned_free(output);
            return -1;
        }
    }

    double end_time = get_time_ms(); /**< Marca de tiempo final */

    result->execution_time_ms =
        end_time - start_time; /**< Cálculo del tiempo de ejecución */

    double total_samples =
        (double) dataset->signal_size *
        (double) dataset->num_filters; /**< Total de muestras procesadas */

    result->throughput =
        total_samples /
        (result->execution_time_ms / 1000.0); /**< Cálculo del rendimiento */

    strcpy(result->dataset_name,
           dataset_name); /**< Copia el nombre del dataset */

    result->implementation =
        IMPLEMENTATION_SCALAR; /**< Marca la implementación utilizada */

    aligned_free(output); /**< Libera el buffer de salida */

    return 0;
}
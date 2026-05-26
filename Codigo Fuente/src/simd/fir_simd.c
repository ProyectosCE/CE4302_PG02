#include "../../include/fir_simd.h"
#include "../../include/aligned_memory.h"

#include <immintrin.h>

#include <stdio.h>
#include <string.h>

/**
 * @file fir_simd.c
 * @brief Implementación SIMD AVX2/FMA del filtro FIR.
 */

/**
 * @brief Ejecuta un filtro FIR SIMD sobre una señal.
 *
 * Procesa 8 muestras simultáneamente utilizando
 * registros AVX2 de 256 bits.
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
)
{
    for (size_t n = 0; n < signal_size; n++)
    {

        /**
         * @brief Preparar Datos: Inicializa el acumulador SIMD a cero usando AVX2.
         * @details
         * Utiliza _mm256_setzero_ps para crear un registro de 256 bits (8 floats) con ceros.
         * Este registro servirá como acumulador para la suma de productos.
         */
        __m256 acc_vec = _mm256_setzero_ps();

        size_t k = 0;

        /**
         * Procesamiento SIMD AVX2
         * 8 floats simultáneamente.
         */

        /**
         * @section Preparar Datos
         * Carga los coeficientes y muestras de señal en registros AVX2 de 256 bits.
         * Se procesan 8 elementos a la vez.
         */
        for (; k + 7 < filter_order; k += 8)
        {
            /**
             * @brief Verifica límites de la señal para evitar accesos fuera de rango.
             * Si n < k + 7, no hay suficientes muestras previas para procesar 8 elementos.
             */
            if (n < k + 7)
            {
                break;
            }

            /**
             * @brief Preparar Datos: Carga 8 coeficientes FIR en un registro AVX2.
             * @details
             * _mm256_loadu_ps carga 8 floats contiguos desde memoria no alineada.
             */
            __m256 filter_vec = _mm256_loadu_ps(&filter[k]);

            /**
             * @brief Preparar Datos: Carga 8 muestras de la señal en orden inverso.
             * @details
             * _mm256_set_ps permite especificar cada float individualmente, cargando las muestras necesarias para la operación FIR.
             */
            __m256 signal_vec = _mm256_set_ps(
                signal[n - (k + 7)],
                signal[n - (k + 6)],
                signal[n - (k + 5)],
                signal[n - (k + 4)],
                signal[n - (k + 3)],
                signal[n - (k + 2)],
                signal[n - (k + 1)],
                signal[n - (k + 0)]
            );

            /**
             * @section Operaciones
             * @brief Realiza la operación Fused Multiply-Add (FMA) sobre los registros AVX2.
             * @details
             * _mm256_fmadd_ps realiza (filter_vec * signal_vec) + acc_vec en un solo paso,
             * aprovechando la instrucción FMA para mayor eficiencia y precisión.
             */
            acc_vec = _mm256_fmadd_ps(filter_vec, signal_vec, acc_vec);
        }


        /**
         * @section Empaquetar y Guardar
         * @brief Reducción horizontal del acumulador SIMD a un escalar.
         * @details
         * _mm256_storeu_ps almacena los 8 floats del registro AVX2 en un arreglo temporal.
         * Luego, se suman manualmente para obtener el valor acumulado final.
         */
        float partial_sum[8];
        _mm256_storeu_ps(partial_sum, acc_vec);
        float acc =
            partial_sum[0] +
            partial_sum[1] +
            partial_sum[2] +
            partial_sum[3] +
            partial_sum[4] +
            partial_sum[5] +
            partial_sum[6] +
            partial_sum[7];

        /**
         * Tail processing escalar.
         */
        for (; k < filter_order; k++)
        {
            if (n >= k)
            {
                acc +=
                    filter[k] *
                    signal[n - k];
            }
        }

        output[n] = acc;
    }
}

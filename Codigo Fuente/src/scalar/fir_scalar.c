#include "../../include/fir_scalar.h"

/**
 * @file fir_scalar.c
 * @brief Implementación secuencial del filtro FIR.
 */

/**
 * @brief Ejecuta un filtro FIR escalar sobre una señal.
 *
 * Recorre la señal de entrada y aplica el filtro FIR de manera secuencial,
 * acumulando el resultado en cada posición de salida.
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
        float acc = 0.0f; /**< Acumulador de la convolución */

        for (size_t k = 0; k < filter_order; k++)
        {
            if (n >= k)
            {
                acc += filter[k] * signal[n - k];
            }
        }

        output[n] = acc; /**< Resultado final para la muestra n */
    }
}
#ifndef FIR_GPU_H
#define FIR_GPU_H

#include <stddef.h>

/**
 * @file fir_gpu.h
 * @brief Interfaz de la implementación FIR en GPU usando OpenCL.
 */

/**
 * @brief Ejecuta un banco de filtros FIR en GPU.
 *
 * La salida se almacena como:
 *
 * output[filter_id * signal_size + sample_id]
 *
 * @param signal Señal de entrada.
 * @param filters Banco de filtros aplanado.
 * @param output Buffer de salida.
 * @param signal_size Tamaño de la señal.
 * @param filter_order Orden de cada filtro.
 * @param filter_count Cantidad de filtros.
 * @param kernel_time_ms Tiempo puro del kernel GPU.
 *
 * @return int 0 si es exitoso, -1 si falla.
 */
int fir_gpu(
    const float* signal,
    const float* filters,
    float* output,
    size_t signal_size,
    size_t filter_order,
    size_t filter_count,
    double* kernel_time_ms
);

#endif
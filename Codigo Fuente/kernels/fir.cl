/**
 * @file fir.cl
 * @brief Kernel OpenCL para aplicar un banco de filtros FIR en GPU.
 *
 * Este kernel implementa el cálculo de un banco de filtros FIR usando
 * paralelismo masivo sobre GPU. Cada work-item calcula una muestra de
 * salida para un filtro específico.
 *
 * La organización del NDRange es bidimensional:
 *
 * - get_global_id(0): índice de muestra de la señal, n.
 * - get_global_id(1): índice del filtro FIR, f.
 *
 * La salida se almacena de forma aplanada:
 *
 * output[f * signal_size + n]
 *
 * De igual forma, los filtros se almacenan de forma aplanada:
 *
 * filters[f * filter_order + k]
 */

/**
 * @brief Aplica un banco de filtros FIR sobre una señal de entrada.
 *
 * Cada work-item calcula una muestra de salida y[n] para un filtro f.
 * El cálculo corresponde a:
 *
 * y_f[n] = sum_{k=0}^{M-1} h_f[k] * x[n-k]
 *
 * donde:
 *
 * - x[n] es la señal de entrada.
 * - h_f[k] es el coeficiente k del filtro f.
 * - y_f[n] es la salida del filtro f en la muestra n.
 * - M es el orden del filtro FIR.
 *
 * @param signal Señal de entrada almacenada en memoria global.
 * @param filters Banco de filtros FIR almacenado en memoria global.
 * @param output Buffer de salida almacenado en memoria global.
 * @param signal_size Tamaño total de la señal de entrada.
 * @param filter_order Orden de cada filtro FIR.
 * @param filter_count Cantidad total de filtros FIR.
 */
__kernel void fir_filter_bank(
    __global const float* signal,
    __global const float* filters,
    __global float* output,
    const int signal_size,
    const int filter_order,
    const int filter_count
)
{
    /**
     * @brief Identificación del work-item actual.
     *
     * n identifica la muestra de la señal que será procesada.
     * f identifica el filtro FIR que será aplicado.
     *
     * Como el kernel se lanza con un NDRange 2D:
     *
     * - La dimensión 0 representa muestras.
     * - La dimensión 1 representa filtros.
     */
    int n = get_global_id(0);
    int f = get_global_id(1);

    /**
     * @brief Protección contra work-items de relleno.
     *
     * El tamaño global puede redondearse hacia arriba para que sea
     * múltiplo del tamaño local del work-group. Por eso pueden existir
     * work-items adicionales fuera del rango real de datos.
     *
     * Si el índice de muestra o filtro excede los límites reales,
     * el work-item termina sin escribir resultados.
     */
    if (n >= signal_size || f >= filter_count)
    {
        return;
    }

    /**
     * @brief Acumulador escalar del producto convolutivo FIR.
     *
     * Cada work-item mantiene su propio acumulador privado.
     */
    float acc = 0.0f;

    /**
     * @brief Cálculo de desplazamientos en arreglos aplanados.
     *
     * filter_base apunta al inicio de los coeficientes del filtro f.
     * output_base apunta al inicio de la salida correspondiente al filtro f.
     */
    int filter_base = f * filter_order;
    int output_base = f * signal_size;

    /**
     * @brief Cálculo FIR para la muestra n del filtro f.
     *
     * Recorre todos los coeficientes del filtro. La condición n >= k
     * evita acceder a posiciones negativas de la señal cuando se calculan
     * las primeras muestras.
     */
    for (int k = 0; k < filter_order; k++)
    {
        if (n >= k)
        {
            acc += filters[filter_base + k] * signal[n - k];
        }
    }

    /**
     * @brief Almacenamiento del resultado final.
     *
     * El resultado se guarda en el arreglo de salida aplanado,
     * en la región correspondiente al filtro f y muestra n.
     */
    output[output_base + n] = acc;
}
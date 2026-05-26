/**
 * @file fir.cl
 * @brief Kernel OpenCL para aplicar un banco de filtros FIR en GPU.
 *
 * Este archivo contiene el código que se ejecuta directamente en el
 * dispositivo OpenCL, es decir, en la GPU. A diferencia del código host en C,
 * este código no crea contextos, colas ni buffers; únicamente define la
 * función paralela que será ejecutada por muchos work-items.
 *
 * El propósito del kernel es aplicar un banco de filtros FIR sobre una señal
 * de entrada. Cada filtro del banco produce una señal de salida independiente.
 *
 * La operación FIR calculada por cada filtro corresponde a:
 *
 * y_f[n] = sum_{k=0}^{M-1} h_f[k] * x[n-k]
 *
 * Donde:
 *
 * - x[n] representa la señal de entrada.
 * - h_f[k] representa el coeficiente k del filtro f.
 * - y_f[n] representa la muestra n de salida producida por el filtro f.
 * - M representa el orden del filtro FIR.
 *
 * La principal diferencia respecto a la versión escalar es que aquí el cálculo
 * no se realiza muestra por muestra de forma secuencial. En su lugar, OpenCL
 * lanza muchos work-items en paralelo. Cada work-item calcula una única muestra
 * de salida para un filtro específico.
 *
 * Organización del NDRange:
 *
 * El kernel se lanza desde el host usando un NDRange bidimensional:
 *
 * - Dimensión 0: muestras de la señal.
 * - Dimensión 1: filtros del banco FIR.
 *
 * Por lo tanto:
 *
 * - get_global_id(0) obtiene el índice de muestra n.
 * - get_global_id(1) obtiene el índice de filtro f.
 *
 * Esto permite explotar dos niveles de paralelismo:
 *
 * - Paralelismo entre muestras de una misma señal filtrada.
 * - Paralelismo entre filtros diferentes del banco FIR.
 *
 * Layout de memoria:
 *
 * OpenCL recibe punteros lineales en memoria global. Por esa razón, tanto el
 * banco de filtros como la salida se almacenan en arreglos aplanados.
 *
 * El banco de filtros se organiza como:
 *
 * filters[f * filter_order + k]
 *
 * Donde:
 *
 * - f identifica el filtro.
 * - k identifica el coeficiente dentro del filtro.
 *
 * La salida se organiza como:
 *
 * output[f * signal_size + n]
 *
 * Donde:
 *
 * - f identifica el filtro que produjo la salida.
 * - n identifica la muestra calculada.
 *
 * Memoria utilizada:
 *
 * - signal, filters y output residen en memoria global del dispositivo.
 * - acc, n, f, k, filter_base y output_base son variables privadas de cada
 *   work-item.
 *
 * Cada work-item trabaja de forma independiente. No existe comunicación entre
 * work-items porque cada uno escribe una posición distinta de output.
 */

/**
 * @brief Aplica un banco de filtros FIR sobre una señal de entrada.
 *
 * Cada work-item calcula una muestra de salida y_f[n] para un filtro f.
 * El cálculo corresponde al producto convolutivo entre los coeficientes
 * del filtro y las muestras anteriores de la señal:
 *
 * y_f[n] = sum_{k=0}^{M-1} h_f[k] * x[n-k]
 *
 * En las primeras muestras de la señal, no existen valores anteriores para
 * todos los coeficientes del filtro. Por esta razón, se utiliza la condición
 * n >= k antes de acceder a signal[n-k]. Esta condición evita accesos fuera
 * de rango y equivale a asumir que las muestras anteriores al inicio de la
 * señal son cero.
 *
 * @param signal Señal de entrada almacenada en memoria global de la GPU.
 * @param filters Banco de filtros FIR almacenado en memoria global de la GPU.
 * @param output Buffer de salida almacenado en memoria global de la GPU.
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
     * @brief Identificación del work-item actual dentro del NDRange.
     *
     * n identifica la muestra de la señal que será procesada.
     * f identifica el filtro FIR que será aplicado.
     *
     * Como el kernel se lanza con un NDRange 2D:
     *
     * - La dimensión 0 representa muestras.
     * - La dimensión 1 representa filtros.
     *
     * Por ejemplo, el work-item con:
     *
     * - n = 25
     * - f = 3
     *
     * calculará la muestra 25 de la salida generada por el filtro 3.
     */
    int n = get_global_id(0);
    int f = get_global_id(1);

    /**
     * @brief Protección contra work-items de relleno o padding.
     *
     * Desde el host, el tamaño global de la dimensión de muestras puede
     * redondearse hacia arriba para que sea múltiplo del tamaño local del
     * work-group. Esto mejora la organización de ejecución en la GPU, pero
     * puede generar work-items adicionales que no corresponden a muestras
     * reales de la señal.
     *
     * Además, esta validación protege la dimensión de filtros.
     *
     * Si n excede signal_size o f excede filter_count, el work-item no debe
     * leer memoria ni escribir resultados.
     */
    if (n >= signal_size || f >= filter_count)
    {
        return;
    }

    /**
     * @brief Acumulador privado del cálculo FIR.
     *
     * Cada work-item mantiene su propio acumulador. Este valor no se comparte
     * con otros work-items, por lo que no se requieren sincronizaciones ni
     * operaciones atómicas.
     *
     * El acumulador representa la suma parcial:
     *
     * acc = h_f[0] * x[n] +
     *       h_f[1] * x[n-1] +
     *       h_f[2] * x[n-2] + ...
     */
    float acc = 0.0f;

    /**
     * @brief Cálculo del desplazamiento base del filtro actual.
     *
     * El banco de filtros está almacenado como un único arreglo lineal.
     * Cada filtro ocupa filter_order posiciones consecutivas.
     *
     * Por lo tanto, el filtro f inicia en:
     *
     * filter_base = f * filter_order
     *
     * El coeficiente k del filtro f se accede como:
     *
     * filters[filter_base + k]
     */
    int filter_base = f * filter_order;

    /**
     * @brief Cálculo del desplazamiento base de la salida actual.
     *
     * La salida también se almacena como un arreglo lineal.
     * Cada filtro produce signal_size muestras de salida.
     *
     * Por lo tanto, la salida del filtro f inicia en:
     *
     * output_base = f * signal_size
     *
     * La muestra n de la salida del filtro f se almacena como:
     *
     * output[output_base + n]
     */
    int output_base = f * signal_size;

    /**
     * @brief Cálculo del producto convolutivo FIR.
     *
     * Se recorren todos los coeficientes del filtro actual. Para cada
     * coeficiente h_f[k], se multiplica por la muestra correspondiente
     * de la señal x[n-k].
     *
     * La condición n >= k es necesaria porque para las primeras muestras
     * de la señal no existen posiciones anteriores suficientes. Por ejemplo,
     * cuando n = 0 solo se puede usar signal[0]. Cuando n = 1 se pueden usar
     * signal[1] y signal[0], y así sucesivamente.
     *
     * En términos prácticos, esto implementa una convolución FIR causal con
     * padding implícito de ceros antes del inicio de la señal.
     */
    for (int k = 0; k < filter_order; k++)
    {
        if (n >= k)
        {
            /**
             * @brief Operación multiply-accumulate del FIR.
             *
             * Equivale a:
             *
             * acc = acc + h_f[k] * x[n-k]
             *
             * Esta operación se repite para todos los coeficientes válidos.
             */
            acc += filters[filter_base + k] * signal[n - k];
        }
    }

    /**
     * @brief Escritura del resultado final en memoria global.
     *
     * Cada work-item escribe exactamente una posición de output. Como el par
     * (f, n) es único para cada work-item, no hay condiciones de carrera entre
     * work-items.
     *
     * El resultado corresponde a la muestra n de la señal filtrada por el
     * filtro f.
     */
    output[output_base + n] = acc;
}
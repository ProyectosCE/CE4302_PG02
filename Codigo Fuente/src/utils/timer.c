#define _POSIX_C_SOURCE 199309L

#include "../../include/timer.h"

#include <time.h>

/**
 * @file timer.c
 * @brief Implementación de funciones para medición de tiempo en milisegundos.
 */

/**
 * @brief Obtiene el tiempo actual en milisegundos usando CLOCK_MONOTONIC.
 *
 * Utiliza clock_gettime para obtener el tiempo transcurrido desde un punto de referencia,
 * retornando el valor en milisegundos. Es útil para medir el rendimiento de algoritmos.
 *
 * @return double Tiempo en milisegundos.
 */
double get_time_ms(void)
{
    struct timespec ts; /**< Estructura para almacenar el tiempo */

    clock_gettime(CLOCK_MONOTONIC, &ts); /**< Obtención del tiempo actual */

    /* Conversión a milisegundos */
    return (ts.tv_sec * 1000.0) +
           (ts.tv_nsec / 1000000.0);
}
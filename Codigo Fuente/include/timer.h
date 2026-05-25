#ifndef TIMER_H
#define TIMER_H

/**
 * @file timer.h
 * @brief Funciones para medición de tiempo de ejecución en milisegundos.
 */

/**
 * @brief Obtiene el tiempo actual en milisegundos.
 *
 * Esta función retorna el tiempo transcurrido desde un punto de referencia (por lo general, desde que el programa inició)
 * en milisegundos, útil para medir el rendimiento de algoritmos.
 *
 * @return double Tiempo en milisegundos.
 */
double get_time_ms(void);

#endif
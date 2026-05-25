#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file common.h
 * @brief Definiciones comunes, tipos y macros para el proyecto de filtrado de señales.
 */

/**
 * @def ALIGNMENT
 * @brief Alineamiento de memoria en bytes para operaciones SIMD o acceso eficiente.
 */
#define ALIGNMENT         32

/**
 * @def NUM_FILTERS
 * @brief Número de filtros FIR por defecto en el dataset.
 */
#define NUM_FILTERS       16

/**
 * @def MAX_PATH_LENGTH
 * @brief Longitud máxima permitida para rutas de archivos.
 */
#define MAX_PATH_LENGTH   256

/**
 * @enum implementation_t
 * @brief Tipos de implementación disponibles para el filtrado FIR.
 */
typedef enum
{
    IMPLEMENTATION_SCALAR = 0, /**< Implementación escalar (CPU, sin vectorización) */
    IMPLEMENTATION_SIMD,       /**< Implementación vectorizada con SIMD */
    IMPLEMENTATION_GPU         /**< Implementación en GPU usando OpenCL */

} implementation_t;

/**
 * @struct dataset_t
 * @brief Estructura que almacena los datos de entrada y filtros para el procesamiento FIR.
 */
typedef struct
{
    float* signal;        /**< Puntero al arreglo de la señal de entrada */
    float* filters;       /**< Puntero al arreglo de coeficientes de los filtros */

    size_t signal_size;   /**< Tamaño de la señal de entrada */
    size_t filter_order;  /**< Orden de los filtros FIR */
    size_t num_filters;   /**< Número de filtros FIR */

} dataset_t;

/**
 * @struct benchmark_result_t
 * @brief Estructura para almacenar los resultados de benchmarks de cada implementación.
 */
typedef struct
{
    double execution_time_ms;      /**< Tiempo de ejecución en milisegundos */
    double throughput;             /**< Rendimiento (elementos procesados por segundo, por ejemplo) */

    char dataset_name[64];         /**< Nombre del dataset utilizado en la prueba */

    implementation_t implementation; /**< Tipo de implementación utilizada */

} benchmark_result_t;

#endif
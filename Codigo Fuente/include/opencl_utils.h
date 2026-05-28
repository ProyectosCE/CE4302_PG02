#ifndef OPENCL_UTILS_H
#define OPENCL_UTILS_H

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>
#include <stddef.h>

/**
 * @file opencl_utils.h
 * @brief Declaración de utilidades auxiliares para OpenCL.
 *
 * Estas funciones centralizan operaciones comunes utilizadas por la
 * implementación GPU:
 *
 * - cálculo de tamaños globales compatibles con work-groups;
 * - verificación de errores OpenCL;
 * - carga de kernels desde archivos .cl;
 * - visualización del log de compilación del driver.
 */

/**
 * @brief Redondea un valor hacia arriba al múltiplo indicado.
 *
 * @param value Valor original.
 * @param multiple Múltiplo deseado.
 *
 * @return size_t Valor redondeado hacia arriba.
 */
size_t opencl_round_up(
    size_t value,
    size_t multiple
);

/**
 * @brief Verifica si un código de error OpenCL indica fallo.
 *
 * @param error Código retornado por OpenCL.
 * @param message Contexto del error.
 *
 * @return int 0 si no hubo error, -1 si hubo error.
 */
int opencl_check_error(
    cl_int error,
    const char* message
);

/**
 * @brief Carga un archivo .cl como una cadena C.
 *
 * @param filename Ruta del archivo del kernel.
 *
 * @return char* Código fuente cargado o NULL si falla.
 */
char* opencl_load_kernel_source(
    const char* filename
);

/**
 * @brief Imprime el log de compilación de un programa OpenCL.
 *
 * @param program Programa OpenCL.
 * @param device Dispositivo OpenCL.
 */
void opencl_print_build_log(
    cl_program program,
    cl_device_id device
);

double opencl_get_event_time_ms(cl_event event);

#endif
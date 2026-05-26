#ifndef OPENCL_UTILS_H
#define OPENCL_UTILS_H

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>
#include <stddef.h>

/**
 * @file opencl_utils.h
 * @brief Utilidades auxiliares para OpenCL.
 */

/**
 * @brief Redondea un valor hacia arriba al múltiplo indicado.
 *
 * @param value Valor original.
 * @param multiple Múltiplo deseado.
 *
 * @return size_t Valor redondeado.
 */
size_t opencl_round_up(
    size_t value,
    size_t multiple
);

/**
 * @brief Verifica un código de error OpenCL.
 *
 * @param error Código retornado por OpenCL.
 * @param message Mensaje contextual.
 *
 * @return int 0 si no hubo error, -1 si hubo error.
 */
int opencl_check_error(
    cl_int error,
    const char* message
);

/**
 * @brief Carga el código fuente de un kernel OpenCL desde archivo.
 *
 * @param filename Ruta del archivo .cl.
 *
 * @return char* Código fuente cargado, o NULL si falla.
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

#endif
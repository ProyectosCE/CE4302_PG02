#ifndef VALIDATION_H
#define VALIDATION_H

#include <stddef.h>

/**
 * @file validation.h
 * @brief Funciones para validación y almacenamiento de resultados en formato binario.
 */

/**
 * @brief Guarda un arreglo de datos en formato binario.
 *
 * Esta función almacena un arreglo de datos de tipo float en un archivo binario especificado por la ruta.
 *
 * @param output_path Ruta del archivo de salida donde se guardarán los datos.
 * @param data Puntero al arreglo de datos a guardar.
 * @param size Cantidad de elementos (floats) a guardar en el archivo.
 * @return int Devuelve 0 si la operación fue exitosa, o un valor negativo si ocurrió un error.
 */
int save_output_binary(
    const char* output_path, /**< Ruta del archivo de salida */
    const float* data,       /**< Puntero al arreglo de datos */
    size_t size              /**< Cantidad de elementos a guardar */
);

#endif
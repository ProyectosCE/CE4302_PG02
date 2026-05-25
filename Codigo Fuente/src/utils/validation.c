#include "../../include/validation.h"

#include <stdio.h>

/**
 * @file validation.c
 * @brief Implementación de funciones para validación y almacenamiento de resultados en binario.
 */

/**
 * @brief Guarda un arreglo de datos tipo float en un archivo binario.
 *
 * Abre el archivo de salida en modo binario y escribe el contenido del arreglo data.
 * Si ocurre un error al crear el archivo, se reporta por stderr.
 *
 * @param output_path Ruta del archivo de salida.
 * @param data Puntero al arreglo de datos a guardar.
 * @param size Cantidad de elementos a guardar.
 * @return int 0 si la operación fue exitosa, -1 si hubo error.
 */
int save_output_binary(
    const char* output_path, /**< Ruta del archivo de salida */
    const float* data,       /**< Puntero al arreglo de datos */
    size_t size              /**< Cantidad de elementos a guardar */
)
{
    FILE* file = fopen(output_path, "wb"); /**< Apertura del archivo en modo binario */

    if (!file)
    {
        fprintf(stderr,
                "Error creating output file.\n");
        return -1;
    }

    /* Escritura de los datos en el archivo */
    fwrite(data,
           sizeof(float),
           size,
           file);

    fclose(file); /**< Cierre del archivo */

    return 0;
}
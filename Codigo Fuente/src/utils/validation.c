#include "../../include/validation.h"

#include <stdio.h>
#include <string.h>

/**
 * @file validation.c
 * @brief Implementación de funciones para almacenamiento de resultados.
 */

/**
 * @brief Guarda un arreglo float en formato binario.
 *
 * La escritura se realiza utilizando fwrite() para minimizar overhead
 * y evitar conversiones innecesarias.
 *
 * @param output_path Ruta del archivo de salida.
 * @param data Puntero a los datos float.
 * @param size Cantidad de elementos float.
 *
 * @return int
 * - 0 si la operación fue exitosa.
 * - -1 si ocurrió un error.
 */
int save_output_binary(
    const char* output_path,
    const float* data,
    size_t size
)
{
    FILE* file = fopen(output_path, "wb");

    if (!file)
    {
        fprintf(stderr,
                "Error: could not create output file: %s\n",
                output_path);

        return -1;
    }

    size_t written = fwrite(
        data,
        sizeof(float),
        size,
        file
    );

    fclose(file);

    if (written != size)
    {
        fprintf(stderr,
                "Error: incomplete write on file: %s\n",
                output_path);

        return -1;
    }

    return 0;
}

/**
 * @brief Construye automáticamente la ruta del archivo de salida.
 *
 * Formato generado:
 *
 * results/<implementation>/output_signal/<dataset>_filter_<id>.bin
 *
 * @param output_path Buffer destino.
 * @param max_length Tamaño máximo del buffer.
 * @param implementation Nombre de implementación.
 * @param dataset_name Nombre del dataset.
 * @param filter_id Número del filtro.
 */
void build_output_path(
    char* output_path,
    size_t max_length,
    const char* implementation,
    const char* dataset_name,
    int filter_id
)
{
    snprintf(
        output_path,
        max_length,
        "results/%s/output_signal/%s_filter_%02d.bin",
        implementation,
        dataset_name,
        filter_id
    );
}
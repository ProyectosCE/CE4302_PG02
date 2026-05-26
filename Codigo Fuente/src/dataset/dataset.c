#include "../../include/dataset.h"
#include "../../include/aligned_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Parsea el archivo de configuración del dataset (declaración interna).
 *
 * Lee los parámetros SIGNAL_SIZE, FILTER_ORDER y NUM_FILTERS del archivo de configuración.
 *
 * @param config_path Ruta al archivo de configuración.
 * @param dataset Puntero a la estructura dataset_t donde se almacenarán los parámetros.
 * @return int 0 si el parseo fue exitoso, -1 si hubo error.
 */
static int parse_config(
    const char* config_path,
    dataset_t* dataset
);

/**
 * @brief Carga datos binarios desde un archivo a un buffer (declaración interna).
 *
 * Abre el archivo en modo binario y lee size_bytes bytes en el buffer proporcionado.
 *
 * @param file_path Ruta al archivo binario.
 * @param buffer Doble puntero donde se almacenarán los datos leídos.
 * @param size_bytes Cantidad de bytes a leer.
 * @return int 0 si la carga fue exitosa, -1 si hubo error.
 */
static int load_binary_file(
    const char* file_path,
    void** buffer,
    size_t size_bytes
);

int load_dataset(const char* dataset_path, dataset_t* dataset)
{
    char config_path[MAX_PATH_LENGTH]; /**< Ruta al archivo de configuración */
    char signal_path[MAX_PATH_LENGTH]; /**< Ruta al archivo de señal */
    char filters_path[MAX_PATH_LENGTH]; /**< Ruta al archivo de filtros */

    /* Construcción de rutas a los archivos */
    snprintf(config_path,
             MAX_PATH_LENGTH,
             "%s/config.txt",
             dataset_path);

    snprintf(signal_path,
             MAX_PATH_LENGTH,
             "%s/signal.bin",
             dataset_path);

    snprintf(filters_path,
             MAX_PATH_LENGTH,
             "%s/filters.bin",
             dataset_path);

    /* Parseo del archivo de configuración */
    if (parse_config(config_path, dataset) != 0)
    {
        return -1;
    }

    size_t signal_size_bytes =
        dataset->signal_size * sizeof(float); /**< Tamaño en bytes de la señal */

    size_t filters_size_bytes =
        dataset->num_filters *
        dataset->filter_order *
        sizeof(float); /**< Tamaño en bytes de los filtros */

    /* Reserva de memoria alineada para la señal y los filtros */
    dataset->signal =
        (float*) aligned_malloc(signal_size_bytes);

    dataset->filters =
        (float*) aligned_malloc(filters_size_bytes);

    if (!dataset->signal || !dataset->filters)
    {
        return -1;
    }

    /* Carga de los datos binarios de la señal */
    if (load_binary_file(signal_path,
                         (void**) &dataset->signal,
                         signal_size_bytes) != 0)
    {
        return -1;
    }

    /* Carga de los datos binarios de los filtros */
    if (load_binary_file(filters_path,
                         (void**) &dataset->filters,
                         filters_size_bytes) != 0)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief Libera la memoria asociada a un dataset.
 *
 * Libera los recursos dinámicos asociados a la estructura dataset_t.
 *
 * @param dataset Puntero a la estructura dataset_t a liberar.
 */
void free_dataset(dataset_t* dataset)
{
    if (dataset->signal)
    {
        aligned_free(dataset->signal); /**< Libera la señal */
    }

    if (dataset->filters)
    {
        aligned_free(dataset->filters); /**< Libera los filtros */
    }
}

/**
 * @brief Imprime información relevante del dataset.
 *
 * Muestra por consola detalles como el tamaño de la señal, orden de los filtros y cantidad de filtros.
 *
 * @param dataset Puntero constante a la estructura dataset_t a mostrar.
 */
void print_dataset_info(const dataset_t* dataset)
{
    printf("Dataset Information\n");
    printf("-------------------\n");

    printf("Signal Size  : %zu\n", dataset->signal_size); /**< Tamaño de la señal */
    printf("Filter Order : %zu\n", dataset->filter_order); /**< Orden de los filtros */
    printf("Num Filters  : %zu\n", dataset->num_filters); /**< Número de filtros */
}

/**
 * @brief Parsea el archivo de configuración del dataset.
 *
 * Lee los parámetros SIGNAL_SIZE, FILTER_ORDER y NUM_FILTERS del archivo de configuración.
 *
 * @param config_path Ruta al archivo de configuración.
 * @param dataset Puntero a la estructura dataset_t donde se almacenarán los parámetros.
 * @return int 0 si el parseo fue exitoso, -1 si hubo error.
 */
static int parse_config(
    const char* config_path,
    dataset_t* dataset
)
{
    FILE* file = fopen(config_path, "r"); /**< Apertura del archivo de configuración */

    if (!file)
    {
        fprintf(stderr,
                "Error opening config file: %s\n",
                config_path);
        return -1;
    }

    char line[128];

    while (fgets(line, sizeof(line), file))
    {
        if (sscanf(line,
                   "SIGNAL_SIZE=%zu",
                   &dataset->signal_size) == 1)
        {
            continue;
        }

        if (sscanf(line,
                   "FILTER_ORDER=%zu",
                   &dataset->filter_order) == 1)
        {
            continue;
        }

        if (sscanf(line,
                   "NUM_FILTERS=%zu",
                   &dataset->num_filters) == 1)
        {
            continue;
        }
    }

    fclose(file); /**< Cierre del archivo de configuración */

    return 0;
}

/**
 * @brief Carga datos binarios desde un archivo a un buffer.
 *
 * Abre el archivo en modo binario y lee size_bytes bytes en el buffer proporcionado.
 *
 * @param file_path Ruta al archivo binario.
 * @param buffer Doble puntero donde se almacenarán los datos leídos.
 * @param size_bytes Cantidad de bytes a leer.
 * @return int 0 si la carga fue exitosa, -1 si hubo error.
 */
static int load_binary_file(
    const char* file_path,
    void** buffer,
    size_t size_bytes
)
{
    FILE* file = fopen(file_path, "rb"); /**< Apertura del archivo binario */

    if (!file)
    {
        fprintf(stderr,
                "Error opening binary file: %s\n",
                file_path);
        return -1;
    }

    size_t read_bytes =
    fread(*buffer, 1, size_bytes, file); /**< Lectura de los datos binarios */

    if (read_bytes != size_bytes)
    {
        fclose(file);
        return -1;
    }

    fclose(file); /**< Cierre del archivo binario */

    return 0;
}
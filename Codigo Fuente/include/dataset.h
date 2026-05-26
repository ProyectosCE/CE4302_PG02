#ifndef DATASET_H
#define DATASET_H

#include "common.h"

/**
 * @file dataset.h
 * @brief Funciones para la gestión y manipulación de datasets de señales y filtros.
 */

/**
 * @brief Carga un dataset desde un archivo de configuración.
 *
 * Lee los datos de señal y filtros desde el archivo especificado y los almacena en la estructura dataset_t.
 *
 * @param dataset_path Ruta al archivo de configuración del dataset.
 * @param dataset Puntero a la estructura dataset_t donde se almacenarán los datos cargados.
 * @return int 0 si la carga fue exitosa, valor negativo si ocurrió un error.
 */
int load_dataset(const char* dataset_path, dataset_t* dataset);

/**
 * @brief Libera la memoria asociada a un dataset.
 *
 * Libera los recursos dinámicos asociados a la estructura dataset_t.
 *
 * @param dataset Puntero a la estructura dataset_t a liberar.
 */
void free_dataset(dataset_t* dataset);

/**
 * @brief Imprime información relevante del dataset.
 *
 * Muestra por consola detalles como el tamaño de la señal, orden de los filtros y cantidad de filtros.
 *
 * @param dataset Puntero constante a la estructura dataset_t a mostrar.
 */
void print_dataset_info(const dataset_t* dataset);

#endif
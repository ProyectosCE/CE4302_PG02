#define _POSIX_C_SOURCE 200112L

#include "../../include/aligned_memory.h"
#include "../../include/common.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * @file aligned_memory.c
 * @brief Implementación de funciones para gestión de memoria alineada.
 */

/**
 * @brief Reserva memoria alineada a un múltiplo de ALIGNMENT.
 *
 * Utiliza posix_memalign para garantizar la alineación requerida por operaciones SIMD o acceso eficiente.
 * Si la asignación falla, se reporta por stderr.
 *
 * @param size Tamaño en bytes a reservar.
 * @return void* Puntero a la memoria alineada, o NULL si falla.
 */
void* aligned_malloc(size_t size)
{
    void* ptr = NULL; /**< Puntero donde se almacenará la dirección alineada */

    int status = posix_memalign(&ptr, ALIGNMENT, size); /**< Solicitud de memoria alineada */

    if (status != 0)
    {
        fprintf(stderr, "Error: aligned allocation failed.\n");
        return NULL;
    }

    return ptr;
}

/**
 * @brief Libera memoria previamente reservada con aligned_malloc.
 *
 * @param ptr Puntero a la memoria alineada a liberar.
 */
void aligned_free(void* ptr)
{
    free(ptr);
}
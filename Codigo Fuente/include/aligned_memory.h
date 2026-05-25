#ifndef ALIGNED_MEMORY_H
#define ALIGNED_MEMORY_H

#include <stddef.h>

/**
 * @file aligned_memory.h
 * @brief Funciones para gestión de memoria alineada, útil para operaciones SIMD y acceso eficiente.
 */

/**
 * @brief Reserva memoria alineada a un múltiplo específico (por ejemplo, 32 bytes).
 *
 * Esta función permite reservar memoria alineada, lo cual es fundamental para operaciones vectorizadas (SIMD) y para evitar penalizaciones de acceso en arquitecturas modernas.
 *
 * @param size Tamaño en bytes a reservar.
 * @return void* Puntero a la memoria alineada reservada, o NULL si falla la asignación.
 */
void* aligned_malloc(size_t size);

/**
 * @brief Libera memoria previamente reservada con aligned_malloc.
 *
 * @param ptr Puntero a la memoria alineada a liberar.
 */
void aligned_free(void* ptr);

#endif
#ifndef ALIGNED_MEMORY_H
#define ALIGNED_MEMORY_H

#include <stddef.h>

void* aligned_malloc(size_t size);

void aligned_free(void* ptr);

#endif
#define _POSIX_C_SOURCE 200112L

#include "../../include/aligned_memory.h"
#include "../../include/common.h"

#include <stdlib.h>
#include <stdio.h>

void* aligned_malloc(size_t size)
{
    void* ptr = NULL;

    int status = posix_memalign(&ptr, ALIGNMENT, size);

    if (status != 0)
    {
        fprintf(stderr, "Error: aligned allocation failed.\n");
        return NULL;
    }

    return ptr;
}

void aligned_free(void* ptr)
{
    free(ptr);
}
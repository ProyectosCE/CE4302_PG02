#ifndef OCL_UTILS_H
#define OCL_UTILS_H

#include <stdio.h>
#include <stdlib.h>

static char* load_kernel_source(const char* filename)
{
    FILE* fp = fopen(filename, "rb");

    if (!fp)
    {
        fprintf(stderr, "Error al abrir el archivo del kernel: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Error al mover el puntero del archivo: %s\n", filename);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    long size = ftell(fp);

    if (size < 0)
    {
        fprintf(stderr, "Error al obtener el tamaño del archivo: %s\n", filename);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    rewind(fp);

    char* source = (char*)malloc((size_t)size + 1);

    if (!source)
    {
        fprintf(stderr, "Error reservando memoria para el kernel.\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(
        source,
        1,
        (size_t)size,
        fp
    );

    if (bytes_read != (size_t)size)
    {
        fprintf(stderr, "Error leyendo el archivo del kernel: %s\n", filename);
        free(source);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    source[size] = '\0';

    fclose(fp);

    return source;
}

#endif
#include "../../include/validation.h"

#include <stdio.h>

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
                "Error creating output file.\n");

        return -1;
    }

    fwrite(data,
           sizeof(float),
           size,
           file);

    fclose(file);

    return 0;
}
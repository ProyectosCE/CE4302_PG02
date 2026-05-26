#define CL_TARGET_OPENCL_VERSION 300

#include "../../include/opencl_utils.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @file opencl_utils.c
 * @brief Implementación de utilidades auxiliares para OpenCL.
 */

size_t opencl_round_up(
    size_t value,
    size_t multiple
)
{
    return ((value + multiple - 1) / multiple) * multiple;
}

int opencl_check_error(
    cl_int error,
    const char* message
)
{
    if (error != CL_SUCCESS)
    {
        fprintf(stderr,
                "OpenCL error at %s: %d\n",
                message,
                error);

        return -1;
    }

    return 0;
}

char* opencl_load_kernel_source(
    const char* filename
)
{
    FILE* file = fopen(filename, "rb");

    if (!file)
    {
        fprintf(stderr,
                "Error: could not open OpenCL kernel file: %s\n",
                filename);

        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(stderr,
                "Error: could not seek OpenCL kernel file: %s\n",
                filename);

        fclose(file);

        return NULL;
    }

    long file_size = ftell(file);

    if (file_size < 0)
    {
        fprintf(stderr,
                "Error: could not get OpenCL kernel file size: %s\n",
                filename);

        fclose(file);

        return NULL;
    }

    rewind(file);

    char* source = (char*)malloc((size_t)file_size + 1);

    if (!source)
    {
        fprintf(stderr,
                "Error: could not allocate memory for OpenCL kernel source.\n");

        fclose(file);

        return NULL;
    }

    size_t bytes_read = fread(
        source,
        1,
        (size_t)file_size,
        file
    );

    fclose(file);

    if (bytes_read != (size_t)file_size)
    {
        fprintf(stderr,
                "Error: incomplete read of OpenCL kernel file: %s\n",
                filename);

        free(source);

        return NULL;
    }

    source[file_size] = '\0';

    return source;
}

void opencl_print_build_log(
    cl_program program,
    cl_device_id device
)
{
    size_t log_size = 0;

    clGetProgramBuildInfo(
        program,
        device,
        CL_PROGRAM_BUILD_LOG,
        0,
        NULL,
        &log_size
    );

    if (log_size == 0)
    {
        return;
    }

    char* log = (char*)malloc(log_size + 1);

    if (!log)
    {
        return;
    }

    clGetProgramBuildInfo(
        program,
        device,
        CL_PROGRAM_BUILD_LOG,
        log_size,
        log,
        NULL
    );

    log[log_size] = '\0';

    fprintf(stderr,
            "OpenCL build log:\n%s\n",
            log);

    free(log);
}
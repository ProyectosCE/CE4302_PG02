#define CL_TARGET_OPENCL_VERSION 300

#include "../../include/fir_gpu.h"
#include "../../include/opencl_utils.h"

#include <CL/cl.h>

#include <stdio.h>
#include <stdlib.h>

#define LOCAL_SIZE_X 64
#define FIR_GPU_KERNEL_PATH "kernels/fir.cl"

/**
 * @file fir_gpu.c
 * @brief Implementación GPU del banco de filtros FIR usando OpenCL.
 */

int fir_gpu(
    const float* signal,
    const float* filters,
    float* output,
    size_t signal_size,
    size_t filter_order,
    size_t filter_count,
    double* kernel_time_ms
)
{
    if (!signal || !filters || !output || !kernel_time_ms)
    {
        fprintf(stderr,
                "Error: invalid argument passed to fir_gpu.\n");

        return -1;
    }

    if (signal_size == 0 || filter_order == 0 || filter_count == 0)
    {
        fprintf(stderr,
                "Error: invalid FIR GPU dimensions.\n");

        return -1;
    }

    cl_int error = CL_SUCCESS;

    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;

    cl_mem signal_mem = NULL;
    cl_mem filters_mem = NULL;
    cl_mem output_mem = NULL;

    cl_event kernel_event = NULL;

    char* kernel_source = NULL;

    int status = -1;

    size_t output_size = signal_size * filter_count;

    size_t global_size[2] =
    {
        opencl_round_up(signal_size, LOCAL_SIZE_X),
        filter_count
    };

    size_t local_size[2] =
    {
        LOCAL_SIZE_X,
        1
    };

    error = clGetPlatformIDs(
        1,
        &platform,
        NULL
    );

    if (opencl_check_error(error, "clGetPlatformIDs") != 0)
    {
        goto cleanup;
    }

    error = clGetDeviceIDs(
        platform,
        CL_DEVICE_TYPE_GPU,
        1,
        &device,
        NULL
    );

    if (opencl_check_error(error, "clGetDeviceIDs") != 0)
    {
        goto cleanup;
    }

    context = clCreateContext(
        NULL,
        1,
        &device,
        NULL,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateContext") != 0)
    {
        goto cleanup;
    }

    cl_command_queue_properties queue_properties[] =
    {
        CL_QUEUE_PROPERTIES,
        CL_QUEUE_PROFILING_ENABLE,
        0
    };

    queue = clCreateCommandQueueWithProperties(
        context,
        device,
        queue_properties,
        &error
    );

    if (opencl_check_error(error, "clCreateCommandQueueWithProperties") != 0)
    {
        goto cleanup;
    }

    signal_mem = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        sizeof(float) * signal_size,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateBuffer signal") != 0)
    {
        goto cleanup;
    }

    filters_mem = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        sizeof(float) * filter_count * filter_order,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateBuffer filters") != 0)
    {
        goto cleanup;
    }

    output_mem = clCreateBuffer(
        context,
        CL_MEM_WRITE_ONLY,
        sizeof(float) * output_size,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateBuffer output") != 0)
    {
        goto cleanup;
    }

    error = clEnqueueWriteBuffer(
        queue,
        signal_mem,
        CL_TRUE,
        0,
        sizeof(float) * signal_size,
        signal,
        0,
        NULL,
        NULL
    );

    if (opencl_check_error(error, "clEnqueueWriteBuffer signal") != 0)
    {
        goto cleanup;
    }

    error = clEnqueueWriteBuffer(
        queue,
        filters_mem,
        CL_TRUE,
        0,
        sizeof(float) * filter_count * filter_order,
        filters,
        0,
        NULL,
        NULL
    );

    if (opencl_check_error(error, "clEnqueueWriteBuffer filters") != 0)
    {
        goto cleanup;
    }

    kernel_source = opencl_load_kernel_source(FIR_GPU_KERNEL_PATH);

    if (!kernel_source)
    {
        goto cleanup;
    }

    program = clCreateProgramWithSource(
        context,
        1,
        (const char**)&kernel_source,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateProgramWithSource") != 0)
    {
        goto cleanup;
    }

    error = clBuildProgram(
        program,
        1,
        &device,
        NULL,
        NULL,
        NULL
    );

    if (error != CL_SUCCESS)
    {
        fprintf(stderr,
                "Error: OpenCL kernel build failed.\n");

        opencl_print_build_log(program, device);

        goto cleanup;
    }

    kernel = clCreateKernel(
        program,
        "fir_filter_bank",
        &error
    );

    if (opencl_check_error(error, "clCreateKernel fir_filter_bank") != 0)
    {
        goto cleanup;
    }

    int signal_size_arg = (int)signal_size;
    int filter_order_arg = (int)filter_order;
    int filter_count_arg = (int)filter_count;

    error = clSetKernelArg(
        kernel,
        0,
        sizeof(cl_mem),
        &signal_mem
    );

    if (opencl_check_error(error, "clSetKernelArg signal") != 0)
    {
        goto cleanup;
    }

    error = clSetKernelArg(
        kernel,
        1,
        sizeof(cl_mem),
        &filters_mem
    );

    if (opencl_check_error(error, "clSetKernelArg filters") != 0)
    {
        goto cleanup;
    }

    error = clSetKernelArg(
        kernel,
        2,
        sizeof(cl_mem),
        &output_mem
    );

    if (opencl_check_error(error, "clSetKernelArg output") != 0)
    {
        goto cleanup;
    }

    error = clSetKernelArg(
        kernel,
        3,
        sizeof(int),
        &signal_size_arg
    );

    if (opencl_check_error(error, "clSetKernelArg signal_size") != 0)
    {
        goto cleanup;
    }

    error = clSetKernelArg(
        kernel,
        4,
        sizeof(int),
        &filter_order_arg
    );

    if (opencl_check_error(error, "clSetKernelArg filter_order") != 0)
    {
        goto cleanup;
    }

    error = clSetKernelArg(
        kernel,
        5,
        sizeof(int),
        &filter_count_arg
    );

    if (opencl_check_error(error, "clSetKernelArg filter_count") != 0)
    {
        goto cleanup;
    }

    error = clEnqueueNDRangeKernel(
        queue,
        kernel,
        2,
        NULL,
        global_size,
        local_size,
        0,
        NULL,
        &kernel_event
    );

    if (opencl_check_error(error, "clEnqueueNDRangeKernel") != 0)
    {
        goto cleanup;
    }

    error = clWaitForEvents(
        1,
        &kernel_event
    );

    if (opencl_check_error(error, "clWaitForEvents") != 0)
    {
        goto cleanup;
    }

    cl_ulong kernel_start = 0;
    cl_ulong kernel_end = 0;

    error = clGetEventProfilingInfo(
        kernel_event,
        CL_PROFILING_COMMAND_START,
        sizeof(kernel_start),
        &kernel_start,
        NULL
    );

    if (opencl_check_error(error, "clGetEventProfilingInfo START") != 0)
    {
        goto cleanup;
    }

    error = clGetEventProfilingInfo(
        kernel_event,
        CL_PROFILING_COMMAND_END,
        sizeof(kernel_end),
        &kernel_end,
        NULL
    );

    if (opencl_check_error(error, "clGetEventProfilingInfo END") != 0)
    {
        goto cleanup;
    }

    *kernel_time_ms = (double)(kernel_end - kernel_start) / 1000000.0;

    error = clEnqueueReadBuffer(
        queue,
        output_mem,
        CL_TRUE,
        0,
        sizeof(float) * output_size,
        output,
        0,
        NULL,
        NULL
    );

    if (opencl_check_error(error, "clEnqueueReadBuffer output") != 0)
    {
        goto cleanup;
    }

    status = 0;

cleanup:

    if (kernel_event)
    {
        clReleaseEvent(kernel_event);
    }

    if (kernel)
    {
        clReleaseKernel(kernel);
    }

    if (program)
    {
        clReleaseProgram(program);
    }

    if (signal_mem)
    {
        clReleaseMemObject(signal_mem);
    }

    if (filters_mem)
    {
        clReleaseMemObject(filters_mem);
    }

    if (output_mem)
    {
        clReleaseMemObject(output_mem);
    }

    if (queue)
    {
        clReleaseCommandQueue(queue);
    }

    if (context)
    {
        clReleaseContext(context);
    }

    free(kernel_source);

    return status;
}
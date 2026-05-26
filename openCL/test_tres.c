#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "ocl_utils.h"

#define LOCAL_SIZE_X 64

#define CHECK_CL(error, msg)                                      \
    do                                                            \
    {                                                             \
        if ((error) != CL_SUCCESS)                                \
        {                                                         \
            fprintf(stderr, "%s failed with error %d\n", msg, error); \
            exit(EXIT_FAILURE);                                   \
        }                                                         \
    } while (0)

static size_t round_up(size_t value, size_t multiple)
{
    return ((value + multiple - 1) / multiple) * multiple;
}

static double elapsed_ms(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

void fir_scalar_bank(
    const float* signal,
    const float* filters,
    float* output,
    size_t signal_size,
    size_t filter_order,
    size_t filter_count
)
{
    for (size_t f = 0; f < filter_count; f++)
    {
        const float* filter = filters + f * filter_order;
        float* current_output = output + f * signal_size;

        for (size_t n = 0; n < signal_size; n++)
        {
            float acc = 0.0f;

            for (size_t k = 0; k < filter_order; k++)
            {
                if (n >= k)
                {
                    acc += filter[k] * signal[n - k];
                }
            }

            current_output[n] = acc;
        }
    }
}

int validate_results(
    const float* expected,
    const float* actual,
    size_t total_size,
    float epsilon
)
{
    for (size_t i = 0; i < total_size; i++)
    {
        float diff = fabsf(expected[i] - actual[i]);

        if (diff > epsilon)
        {
            fprintf(stderr,
                    "Validation failed at index %zu: expected=%f actual=%f diff=%f\n",
                    i,
                    expected[i],
                    actual[i],
                    diff);

            return -1;
        }
    }

    return 0;
}

int main(void)
{
    const size_t signal_size = 1000005;
    const size_t filter_order = 128;
    const size_t filter_count = 4;

    const size_t output_size = signal_size * filter_count;

    size_t global_size[2] =
    {
        round_up(signal_size, LOCAL_SIZE_X),
        filter_count
    };

    size_t local_size[2] =
    {
        LOCAL_SIZE_X,
        1
    };

    size_t work_groups_x = global_size[0] / local_size[0];
    size_t total_work_groups = work_groups_x * filter_count;

    printf("Signal size:              %zu\n", signal_size);
    printf("Filter order:             %zu\n", filter_order);
    printf("Filter count:             %zu\n", filter_count);
    printf("Global size X padded:     %zu\n", global_size[0]);
    printf("Padding work-items X:     %zu\n", global_size[0] - signal_size);
    printf("Work-groups per filter:   %zu\n", work_groups_x);
    printf("Total work-groups:        %zu\n\n", total_work_groups);

    float* signal = (float*)malloc(sizeof(float) * signal_size);
    float* filters = (float*)malloc(sizeof(float) * filter_count * filter_order);
    float* output_gpu = (float*)malloc(sizeof(float) * output_size);
    float* output_cpu = (float*)malloc(sizeof(float) * output_size);
    int* wg_ids = (int*)malloc(sizeof(int) * output_size);

    if (!signal || !filters || !output_gpu || !output_cpu || !wg_ids)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    for (size_t n = 0; n < signal_size; n++)
    {
        float t = (float)n;
        signal[n] =
            sinf(0.01f * t) +
            0.5f * sinf(0.05f * t) +
            0.1f * sinf(0.25f * t);
    }

    for (size_t f = 0; f < filter_count; f++)
    {
        for (size_t k = 0; k < filter_order; k++)
        {
            filters[f * filter_order + k] =
                1.0f / ((float)filter_order + (float)f);
        }
    }

    struct timespec cpu_start;
    struct timespec cpu_end;

    clock_gettime(CLOCK_MONOTONIC, &cpu_start);

    fir_scalar_bank(
        signal,
        filters,
        output_cpu,
        signal_size,
        filter_order,
        filter_count
    );

    clock_gettime(CLOCK_MONOTONIC, &cpu_end);

    double cpu_ms = elapsed_ms(cpu_start, cpu_end);

    cl_int err;

    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;

    err = clGetPlatformIDs(1, &platform_id, NULL);
    CHECK_CL(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(
        platform_id,
        CL_DEVICE_TYPE_GPU,
        1,
        &device_id,
        NULL
    );
    CHECK_CL(err, "clGetDeviceIDs");

    cl_uint compute_units = 0;
    clGetDeviceInfo(
        device_id,
        CL_DEVICE_MAX_COMPUTE_UNITS,
        sizeof(compute_units),
        &compute_units,
        NULL
    );

    printf("GPU compute units detected: %u\n", compute_units);

    cl_context context = clCreateContext(
        NULL,
        1,
        &device_id,
        NULL,
        NULL,
        &err
    );
    CHECK_CL(err, "clCreateContext");

    cl_command_queue_properties props[] =
    {
        CL_QUEUE_PROPERTIES,
        CL_QUEUE_PROFILING_ENABLE,
        0
    };

    cl_command_queue queue = clCreateCommandQueueWithProperties(
        context,
        device_id,
        props,
        &err
    );
    CHECK_CL(err, "clCreateCommandQueueWithProperties");

    cl_mem signal_mem = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        sizeof(float) * signal_size,
        NULL,
        &err
    );
    CHECK_CL(err, "clCreateBuffer signal");

    cl_mem filters_mem = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        sizeof(float) * filter_count * filter_order,
        NULL,
        &err
    );
    CHECK_CL(err, "clCreateBuffer filters");

    cl_mem output_mem = clCreateBuffer(
        context,
        CL_MEM_WRITE_ONLY,
        sizeof(float) * output_size,
        NULL,
        &err
    );
    CHECK_CL(err, "clCreateBuffer output");

    cl_mem wg_ids_mem = clCreateBuffer(
        context,
        CL_MEM_WRITE_ONLY,
        sizeof(int) * output_size,
        NULL,
        &err
    );
    CHECK_CL(err, "clCreateBuffer wg_ids");

    struct timespec gpu_total_start;
    struct timespec gpu_total_end;

    clock_gettime(CLOCK_MONOTONIC, &gpu_total_start);

    err = clEnqueueWriteBuffer(
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
    CHECK_CL(err, "clEnqueueWriteBuffer signal");

    err = clEnqueueWriteBuffer(
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
    CHECK_CL(err, "clEnqueueWriteBuffer filters");

    char* kernel_source = load_kernel_source("kernel.cl");

    cl_program program = clCreateProgramWithSource(
        context,
        1,
        (const char**)&kernel_source,
        NULL,
        &err
    );
    CHECK_CL(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    if (err != CL_SUCCESS)
    {
        size_t log_size = 0;

        clGetProgramBuildInfo(
            program,
            device_id,
            CL_PROGRAM_BUILD_LOG,
            0,
            NULL,
            &log_size
        );

        char* log = (char*)malloc(log_size + 1);

        clGetProgramBuildInfo(
            program,
            device_id,
            CL_PROGRAM_BUILD_LOG,
            log_size,
            log,
            NULL
        );

        log[log_size] = '\0';

        fprintf(stderr, "OpenCL build log:\n%s\n", log);

        free(log);
        free(kernel_source);

        return EXIT_FAILURE;
    }

    cl_kernel kernel = clCreateKernel(
        program,
        "fir_filter_bank",
        &err
    );
    CHECK_CL(err, "clCreateKernel");

    int signal_size_arg = (int)signal_size;
    int filter_order_arg = (int)filter_order;
    int filter_count_arg = (int)filter_count;

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &signal_mem);
    CHECK_CL(err, "clSetKernelArg 0");

    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &filters_mem);
    CHECK_CL(err, "clSetKernelArg 1");

    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_mem);
    CHECK_CL(err, "clSetKernelArg 2");

    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &wg_ids_mem);
    CHECK_CL(err, "clSetKernelArg 3");

    err = clSetKernelArg(kernel, 4, sizeof(int), &signal_size_arg);
    CHECK_CL(err, "clSetKernelArg 4");

    err = clSetKernelArg(kernel, 5, sizeof(int), &filter_order_arg);
    CHECK_CL(err, "clSetKernelArg 5");

    err = clSetKernelArg(kernel, 6, sizeof(int), &filter_count_arg);
    CHECK_CL(err, "clSetKernelArg 6");

    cl_event kernel_event;

    err = clEnqueueNDRangeKernel(
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
    CHECK_CL(err, "clEnqueueNDRangeKernel");

    err = clWaitForEvents(1, &kernel_event);
    CHECK_CL(err, "clWaitForEvents");

    cl_ulong kernel_start = 0;
    cl_ulong kernel_end = 0;

    clGetEventProfilingInfo(
        kernel_event,
        CL_PROFILING_COMMAND_START,
        sizeof(kernel_start),
        &kernel_start,
        NULL
    );

    clGetEventProfilingInfo(
        kernel_event,
        CL_PROFILING_COMMAND_END,
        sizeof(kernel_end),
        &kernel_end,
        NULL
    );

    double kernel_ms = (double)(kernel_end - kernel_start) / 1000000.0;

    err = clEnqueueReadBuffer(
        queue,
        output_mem,
        CL_TRUE,
        0,
        sizeof(float) * output_size,
        output_gpu,
        0,
        NULL,
        NULL
    );
    CHECK_CL(err, "clEnqueueReadBuffer output");

    err = clEnqueueReadBuffer(
        queue,
        wg_ids_mem,
        CL_TRUE,
        0,
        sizeof(int) * output_size,
        wg_ids,
        0,
        NULL,
        NULL
    );
    CHECK_CL(err, "clEnqueueReadBuffer wg_ids");

    clock_gettime(CLOCK_MONOTONIC, &gpu_total_end);

    double gpu_total_ms = elapsed_ms(gpu_total_start, gpu_total_end);

    int validation = validate_results(
        output_cpu,
        output_gpu,
        output_size,
        0.001f
    );

    double speedup_kernel = cpu_ms / kernel_ms;
    double speedup_total = cpu_ms / gpu_total_ms;

    double total_samples_processed =
        (double)signal_size * (double)filter_count;

    double throughput_kernel =
        total_samples_processed / (kernel_ms / 1000.0);

    double throughput_total =
        total_samples_processed / (gpu_total_ms / 1000.0);

    printf("\n==================================================\n");
    printf("RESULTADOS FIR CPU vs GPU\n");
    printf("==================================================\n");
    printf("Validación:                 %s\n",
           validation == 0 ? "OK" : "ERROR");
    printf("Tiempo CPU escalar:         %.4f ms\n", cpu_ms);
    printf("Tiempo kernel GPU puro:     %.4f ms\n", kernel_ms);
    printf("Tiempo GPU total:           %.4f ms\n", gpu_total_ms);
    printf("Speedup kernel GPU:         %.4fx\n", speedup_kernel);
    printf("Speedup GPU total:          %.4fx\n", speedup_total);
    printf("Throughput kernel:          %.2f muestras/s\n", throughput_kernel);
    printf("Throughput total:           %.2f muestras/s\n", throughput_total);
    printf("==================================================\n");

    size_t* group_counts = (size_t*)calloc(work_groups_x, sizeof(size_t));

    for (size_t f = 0; f < filter_count; f++)
    {
        for (size_t n = 0; n < signal_size; n++)
        {
            int group_id = wg_ids[f * signal_size + n];

            if (group_id >= 0 && (size_t)group_id < work_groups_x)
            {
                group_counts[group_id]++;
            }
        }
    }

    printf("\nDistribución aproximada por Compute Unit:\n");

    for (cl_uint cu = 0; cu < compute_units; cu++)
    {
        size_t load = 0;

        for (size_t g = cu; g < work_groups_x; g += compute_units)
        {
            load += group_counts[g];
        }

        printf("CU %u: %zu muestras útiles\n", cu, load);
    }

    free(group_counts);

    clReleaseEvent(kernel_event);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(signal_mem);
    clReleaseMemObject(filters_mem);
    clReleaseMemObject(output_mem);
    clReleaseMemObject(wg_ids_mem);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(kernel_source);
    free(signal);
    free(filters);
    free(output_gpu);
    free(output_cpu);
    free(wg_ids);

    return validation == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
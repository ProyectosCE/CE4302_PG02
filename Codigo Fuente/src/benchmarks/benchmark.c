#include "../../include/common.h"
#include "../../include/aligned_memory.h"
#include "../../include/timer.h"
#include "../../include/validation.h"

#if defined(BUILD_SCALAR)
#include "../../include/fir_scalar.h"
#endif

#if defined(BUILD_GPU)
#include "../../include/fir_gpu.h"
#endif

#include <stdio.h>
#include <string.h>

/**
 * @file benchmark.c
 * @brief Benchmark centralizado para implementaciones FIR.
 */

void print_benchmark_result(
    const benchmark_result_t* result
)
{
    printf("\nBenchmark Result\n");
    printf("----------------\n");

    printf("Dataset        : %s\n",
           result->dataset_name);

    printf("Implementation : ");

    switch (result->implementation)
    {
        case IMPLEMENTATION_SCALAR:
            printf("SCALAR\n");
            break;

        case IMPLEMENTATION_SIMD:
            printf("SIMD\n");
            break;

        case IMPLEMENTATION_GPU:
            printf("GPU\n");
            break;

        default:
            printf("UNKNOWN\n");
            break;
    }

    printf("Execution Time : %.3f ms\n",
           result->execution_time_ms);

    printf("Throughput     : %.3f samples/sec\n",
           result->throughput);
}

#if defined(BUILD_SCALAR)

int run_fir_scalar_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
)
{
    size_t output_size_bytes =
        dataset->signal_size * sizeof(float);

    float* output =
        (float*) aligned_malloc(output_size_bytes);

    if (!output)
    {
        return -1;
    }

    double start_time = get_time_ms();

    for (size_t filter_id = 0;
         filter_id < dataset->num_filters;
         filter_id++)
    {
        const float* current_filter =
            &dataset->filters[
                filter_id * dataset->filter_order
            ];

        fir_scalar(
            dataset->signal,
            current_filter,
            output,
            dataset->signal_size,
            dataset->filter_order
        );

        char output_path[MAX_PATH_LENGTH];

        build_output_path(
            output_path,
            MAX_PATH_LENGTH,
            "scalar",
            dataset_name,
            (int) filter_id
        );

        if (save_output_binary(
                output_path,
                output,
                dataset->signal_size) != 0)
        {
            aligned_free(output);

            return -1;
        }
    }

    double end_time = get_time_ms();

    result->execution_time_ms =
        end_time - start_time;

    double total_samples =
        (double) dataset->signal_size *
        (double) dataset->num_filters;

    result->throughput =
        total_samples /
        (result->execution_time_ms / 1000.0);

    strcpy(result->dataset_name,
           dataset_name);

    result->implementation =
        IMPLEMENTATION_SCALAR;

    aligned_free(output);

    return 0;
}

#endif

#if defined(BUILD_GPU)

int run_fir_gpu_benchmark(
    const dataset_t* dataset,
    const char* dataset_name,
    benchmark_result_t* result
)
{
    if (!dataset || !dataset_name || !result)
    {
        fprintf(stderr,
                "Error: invalid argument passed to run_fir_gpu_benchmark.\n");

        return -1;
    }

    const float* signal = dataset->signal;
    const float* filters = dataset->filters;

    size_t signal_size = dataset->signal_size;
    size_t filter_order = dataset->filter_order;
    size_t filter_count = dataset->num_filters;

    size_t output_size =
        signal_size * filter_count;

    float* output =
        (float*) aligned_malloc(sizeof(float) * output_size);

    if (!output)
    {
        fprintf(stderr,
                "Error: could not allocate GPU output buffer.\n");

        return -1;
    }

    double kernel_time_ms = 0.0;

    double start_time = get_time_ms();

    int status = fir_gpu(
        signal,
        filters,
        output,
        signal_size,
        filter_order,
        filter_count,
        &kernel_time_ms
    );

    double end_time = get_time_ms();

    if (status != 0)
    {
        fprintf(stderr,
                "Error: GPU FIR execution failed.\n");

        aligned_free(output);

        return -1;
    }

    result->execution_time_ms =
        end_time - start_time;

    double total_samples =
        (double) signal_size *
        (double) filter_count;

    result->throughput =
        total_samples /
        (result->execution_time_ms / 1000.0);

    strcpy(result->dataset_name,
           dataset_name);

    result->implementation =
        IMPLEMENTATION_GPU;

    for (size_t filter_id = 0;
         filter_id < filter_count;
         filter_id++)
    {
        char output_path[MAX_PATH_LENGTH];

        build_output_path(
            output_path,
            MAX_PATH_LENGTH,
            "gpu",
            dataset_name,
            (int) filter_id
        );

        if (save_output_binary(
                output_path,
                output + filter_id * signal_size,
                signal_size) != 0)
        {
            aligned_free(output);

            return -1;
        }
    }

    printf("\nGPU Details\n");
    printf("-----------\n");
    printf("Kernel Time    : %.3f ms\n",
           kernel_time_ms);
    printf("Kernel Throughput : %.3f samples/sec\n",
           total_samples / (kernel_time_ms / 1000.0));

    aligned_free(output);

    return 0;
}

#endif
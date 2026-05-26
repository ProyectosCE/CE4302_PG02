#include "../../include/gpu_benchmark.h"

#include "../../include/fir_gpu.h"
#include "../../include/aligned_memory.h"
#include "../../include/timer.h"
#include "../../include/validation.h"

#include <stdio.h>
#include <string.h>

/**
 * @file gpu_benchmark.c
 * @brief Benchmark para implementación FIR GPU.
 */

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

    const float* signal =
        dataset->signal;

    const float* filters =
        dataset->filters;

    size_t signal_size =
        dataset->signal_size;

    size_t filter_order =
        dataset->filter_order;

    size_t filter_count =
        dataset->num_filters;

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

    double kernel_time_ms =
        0.0;

    double start_time =
        get_time_ms();

    int status =
        fir_gpu(
            signal,
            filters,
            output,
            signal_size,
            filter_order,
            filter_count,
            &kernel_time_ms
        );

    double end_time =
        get_time_ms();

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

    strcpy(
        result->dataset_name,
        dataset_name
    );

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

    printf("Kernel Time       : %.3f ms\n",
           kernel_time_ms);

    printf("Kernel Throughput : %.3f samples/sec\n",
           total_samples / (kernel_time_ms / 1000.0));

    aligned_free(output);

    return 0;
}
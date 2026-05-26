#include "../include/dataset.h"
#include "../include/common.h"
#include "../include/benchmark_common.h"

#if defined(BUILD_SCALAR)
#include "../include/scalar_benchmark.h"
#endif

#if defined(BUILD_SIMD)
#include "../include/simd_benchmark.h"
#endif

#if defined(BUILD_GPU)
#include "../include/gpu_benchmark.h"
#endif

#include <stdio.h>
#include <string.h>

/**
 * @file main.c
 * @brief Punto de entrada único para scalar, SIMD y GPU.
 */

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr,
                "Usage: %s <dataset>\n",
                argv[0]);

        fprintf(stderr,
                "Available datasets: small, medium, large\n");

        return -1;
    }

    const char* dataset_name =
        argv[1];

    if (strcmp(dataset_name, "small") != 0 &&
        strcmp(dataset_name, "medium") != 0 &&
        strcmp(dataset_name, "large") != 0)
    {
        fprintf(stderr,
                "Invalid dataset: %s\n",
                dataset_name);

        fprintf(stderr,
                "Available datasets: small, medium, large\n");

        return -1;
    }

    char dataset_path[MAX_PATH_LENGTH];

    snprintf(
        dataset_path,
        MAX_PATH_LENGTH,
        "datasets/%s",
        dataset_name
    );

    dataset_t dataset;

    if (load_dataset(
            dataset_path,
            &dataset) != 0)
    {
        fprintf(stderr,
                "Failed loading dataset: %s\n",
                dataset_path);

        return -1;
    }

    print_dataset_info(&dataset);

    benchmark_result_t result;

#if defined(BUILD_SCALAR)

    if (run_fir_scalar_benchmark(
            &dataset,
            dataset_name,
            &result) != 0)
    {
        fprintf(stderr,
                "Scalar benchmark failed.\n");

        free_dataset(&dataset);

        return -1;
    }

#elif defined(BUILD_SIMD)

    if (run_fir_simd_benchmark(
            &dataset,
            dataset_name,
            &result) != 0)
    {
        fprintf(stderr,
                "SIMD benchmark failed.\n");

        free_dataset(&dataset);

        return -1;
    }

#elif defined(BUILD_GPU)

    if (run_fir_gpu_benchmark(
            &dataset,
            dataset_name,
            &result) != 0)
    {
        fprintf(stderr,
                "GPU benchmark failed.\n");

        free_dataset(&dataset);

        return -1;
    }

#else

    fprintf(stderr,
            "Error: no implementation selected at compile time.\n");

    free_dataset(&dataset);

    return -1;

#endif

    print_benchmark_result(&result);

    free_dataset(&dataset);

    return 0;
}
#include "../include/dataset.h"
#include "../include/common.h"

#include "../include/scalar_benchmark.h"
#include "../include/simd_benchmark.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief Función externa para impresión de benchmark.
 */
void print_benchmark_result(
    const benchmark_result_t* result
);

/**
 * @brief Punto de entrada principal para implementación SIMD.
 *
 * Uso:
 *
 * ./build/fir_project <dataset>
 *
 * Ejemplo:
 *
 * ./build/fir_project medium
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

    /* VALIDACION DATASET */
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

    print_benchmark_result(&result);

    free_dataset(&dataset);

    return 0;
}
#include "../include/dataset.h"
#include "../include/fir_scalar.h"
#include "../include/common.h"

#include <stdio.h>

/**
 * @brief Función externa para impresión de benchmark.
 */
void print_benchmark_result(
    const benchmark_result_t* result
);

/**
 * @brief Punto de entrada principal.
 *
 * Uso:
 *
 * ./build/fir_project <dataset>
 *
 * Ejemplo:
 *
 * ./build/fir_project small
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

    char dataset_path[MAX_PATH_LENGTH];

    snprintf(
        dataset_path,
        MAX_PATH_LENGTH,
        "datasets/%s",
        argv[1]
    );

    dataset_t dataset;

    if (load_dataset(dataset_path,
                     &dataset) != 0)
    {
        fprintf(stderr,
                "Failed loading dataset: %s\n",
                dataset_path);

        return -1;
    }

    print_dataset_info(&dataset);

    benchmark_result_t result;

    if (run_fir_scalar_benchmark(
            &dataset,
            argv[1],
            &result) != 0)
    {
        fprintf(stderr,
                "Scalar benchmark failed.\n");

        free_dataset(&dataset);

        return -1;
    }

    print_benchmark_result(&result);

    free_dataset(&dataset);

    return 0;
}
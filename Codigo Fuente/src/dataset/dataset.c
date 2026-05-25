#include "../../include/dataset.h"
#include "../../include/aligned_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_config(
    const char* config_path,
    dataset_t* dataset
);

static int load_binary_file(
    const char* file_path,
    void** buffer,
    size_t size_bytes
);

int load_dataset(const char* dataset_path, dataset_t* dataset)
{
    char config_path[MAX_PATH_LENGTH];
    char signal_path[MAX_PATH_LENGTH];
    char filters_path[MAX_PATH_LENGTH];

    snprintf(config_path,
             MAX_PATH_LENGTH,
             "%s/config.txt",
             dataset_path);

    snprintf(signal_path,
             MAX_PATH_LENGTH,
             "%s/signal.bin",
             dataset_path);

    snprintf(filters_path,
             MAX_PATH_LENGTH,
             "%s/filters.bin",
             dataset_path);

    if (parse_config(config_path, dataset) != 0)
    {
        return -1;
    }

    size_t signal_size_bytes =
        dataset->signal_size * sizeof(float);

    size_t filters_size_bytes =
        dataset->num_filters *
        dataset->filter_order *
        sizeof(float);

    dataset->signal =
        (float*) aligned_malloc(signal_size_bytes);

    dataset->filters =
        (float*) aligned_malloc(filters_size_bytes);

    if (!dataset->signal || !dataset->filters)
    {
        return -1;
    }

    if (load_binary_file(signal_path,
                         (void**) &dataset->signal,
                         signal_size_bytes) != 0)
    {
        return -1;
    }

    if (load_binary_file(filters_path,
                         (void**) &dataset->filters,
                         filters_size_bytes) != 0)
    {
        return -1;
    }

    return 0;
}

void free_dataset(dataset_t* dataset)
{
    if (dataset->signal)
    {
        aligned_free(dataset->signal);
    }

    if (dataset->filters)
    {
        aligned_free(dataset->filters);
    }
}

void print_dataset_info(const dataset_t* dataset)
{
    printf("Dataset Information\n");
    printf("-------------------\n");

    printf("Signal Size  : %zu\n", dataset->signal_size);
    printf("Filter Order : %zu\n", dataset->filter_order);
    printf("Num Filters  : %zu\n", dataset->num_filters);
}

static int parse_config(
    const char* config_path,
    dataset_t* dataset
)
{
    FILE* file = fopen(config_path, "r");

    if (!file)
    {
        fprintf(stderr,
                "Error opening config file: %s\n",
                config_path);

        return -1;
    }

    char line[128];

    while (fgets(line, sizeof(line), file))
    {
        if (sscanf(line,
                   "SIGNAL_SIZE=%zu",
                   &dataset->signal_size) == 1)
        {
            continue;
        }

        if (sscanf(line,
                   "FILTER_ORDER=%zu",
                   &dataset->filter_order) == 1)
        {
            continue;
        }

        if (sscanf(line,
                   "NUM_FILTERS=%zu",
                   &dataset->num_filters) == 1)
        {
            continue;
        }
    }

    fclose(file);

    return 0;
}

static int load_binary_file(
    const char* file_path,
    void** buffer,
    size_t size_bytes
)
{
    FILE* file = fopen(file_path, "rb");

    if (!file)
    {
        fprintf(stderr,
                "Error opening binary file: %s\n",
                file_path);

        return -1;
    }

    size_t read_bytes =
    fread(*buffer, 1, size_bytes, file);

    if (read_bytes != size_bytes)
    {
        fclose(file);
        return -1;
    }

    fclose(file);

    return 0;
}
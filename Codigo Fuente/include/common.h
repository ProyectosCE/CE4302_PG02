#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

#define ALIGNMENT         32
#define NUM_FILTERS       16
#define MAX_PATH_LENGTH   256

typedef enum
{
    IMPLEMENTATION_SCALAR = 0,
    IMPLEMENTATION_SIMD,
    IMPLEMENTATION_GPU

} implementation_t;

typedef struct
{
    float* signal;
    float* filters;

    size_t signal_size;
    size_t filter_order;
    size_t num_filters;

} dataset_t;

typedef struct
{
    double execution_time_ms;
    double throughput;

    char dataset_name[64];

    implementation_t implementation;

} benchmark_result_t;

#endif
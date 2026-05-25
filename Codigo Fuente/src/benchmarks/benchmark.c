#include "../../include/common.h"

#include <stdio.h>

void print_benchmark_result(
    const benchmark_result_t* result
)
{
    printf("\nBenchmark Result\n");
    printf("----------------\n");

    printf("Dataset        : %s\n",
           result->dataset_name);

    printf("Execution Time : %.3f ms\n",
           result->execution_time_ms);

    printf("Throughput     : %.3f samples/sec\n",
           result->throughput);
}
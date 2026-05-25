#include "../include/dataset.h"

#include <stdio.h>

int main(void)
{
    dataset_t dataset;

    if (load_dataset("datasets/small",
                     &dataset) != 0)
    {
        fprintf(stderr,
                "Failed loading dataset.\n");

        return -1;
    }

    print_dataset_info(&dataset);

    free_dataset(&dataset);

    return 0;
}
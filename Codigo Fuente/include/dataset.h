#ifndef DATASET_H
#define DATASET_H

#include "common.h"

int load_dataset(const char* dataset_path, dataset_t* dataset);

void free_dataset(dataset_t* dataset);

void print_dataset_info(const dataset_t* dataset);

#endif
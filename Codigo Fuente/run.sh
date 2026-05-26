#!/bin/bash

# VALIDACION DE ARGUMENTOS
if [ $# -lt 2 ]; then
    echo "Usage: ./run.sh <implementation> <dataset>"
    echo ""
    echo "Implementations:"
    echo "  scalar"
    echo "  simd"
    echo ""
    echo "Datasets:"
    echo "  small"
    echo "  medium"
    echo "  large"
    exit 1
fi

IMPLEMENTATION=$1
DATASET=$2

# VALIDACION IMPLEMENTACION
if [ "$IMPLEMENTATION" != "scalar" ] && \
   [ "$IMPLEMENTATION" != "simd" ]; then

    echo "Invalid implementation: $IMPLEMENTATION"
    echo "Available implementations: scalar, simd"
    exit 1
fi

# VALIDACION DATASET
if [ "$DATASET" != "small" ] && \
   [ "$DATASET" != "medium" ] && \
   [ "$DATASET" != "large" ]; then

    echo "Invalid dataset: $DATASET"
    echo "Available datasets: small, medium, large"
    exit 1
fi

# BUILD
make clean
make $IMPLEMENTATION

# RUN
echo ""
echo "Running FIR Project..."
echo "Implementation : $IMPLEMENTATION"
echo "Dataset        : $DATASET"
echo ""

./build/fir_project $IMPLEMENTATION $DATASET
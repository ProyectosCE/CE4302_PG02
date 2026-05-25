#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: ./run.sh <dataset>"
    echo "Available datasets: small, medium, large"
    exit 1
fi

make clean
make

echo ""
echo "Running FIR Project..."
echo ""

./build/fir_project $1
#!/bin/bash

# FIR ASSEMBLY ANALYSIS SCRIPT
#
# USAGE:
# ./run_asm.sh scalar
# ./run_asm.sh simd
# ./run_asm.sh gpu

# VALIDACION ARGUMENTOS
if [ $# -lt 1 ]; then
    echo "Usage: ./run_asm.sh <implementation>"
    echo ""
    echo "Available implementations:"
    echo "  scalar"
    echo "  simd"
    echo "  gpu"
    exit 1
fi

IMPLEMENTATION=$1

# VALIDACION IMPLEMENTACION
if [[ "$IMPLEMENTATION" != "scalar" &&
      "$IMPLEMENTATION" != "simd" &&
      "$IMPLEMENTATION" != "gpu" ]]; then

    echo "Invalid implementation: $IMPLEMENTATION"
    echo ""
    echo "Available implementations:"
    echo "  scalar"
    echo "  simd"
    echo "  gpu"
    exit 1
fi

# DIRECTORIOS
OUTPUT_DIR="results/${IMPLEMENTATION}/asm_output"

mkdir -p "$OUTPUT_DIR"

# COMPILACION
echo ""
echo "========================================================="
echo "COMPILING IMPLEMENTATION: $IMPLEMENTATION"
echo "========================================================="
echo ""

make clean
make "$IMPLEMENTATION"

if [ $? -ne 0 ]; then
    echo ""
    echo "Compilation failed."
    exit 1
fi

# VALIDAR EJECUTABLE
if [ ! -f "./build/fir_project" ]; then
    echo ""
    echo "Executable not found:"
    echo "./build/fir_project"
    exit 1
fi

# GENERACION ASSEMBLY
echo ""
echo "========================================================="
echo "GENERATING ASSEMBLY"
echo "========================================================="
echo ""

# DESENSAMBLADO COMPLETO
objdump -d -M intel build/fir_project \
> "${OUTPUT_DIR}/full_disassembly.txt"

# EXTRAER FUNCIONES RELEVANTES
if [ "$IMPLEMENTATION" == "gpu" ]; then

    objdump -d -M intel build/fir_project | \
    grep -A 160 "<fir_gpu>\|<opencl_\|<run_fir_gpu_benchmark>" \
    > "${OUTPUT_DIR}/fir_functions_only.txt"

else

    objdump -d -M intel build/fir_project | \
    grep -A 160 "<fir_\|<run_fir_" \
    > "${OUTPUT_DIR}/fir_functions_only.txt"

fi

echo "Assembly files generated:"
echo ""
echo "${OUTPUT_DIR}/full_disassembly.txt"
echo "${OUTPUT_DIR}/fir_functions_only.txt"
echo ""

# BUSQUEDA SIMD
if [ "$IMPLEMENTATION" == "simd" ]; then

    echo "========================================================="
    echo "SEARCHING FOR SIMD INSTRUCTIONS"
    echo "========================================================="
    echo ""

    grep -i "ymm\|xmm\|vfmadd\|vmulps\|vaddps\|vmovups" \
    "${OUTPUT_DIR}/full_disassembly.txt" \
    > "${OUTPUT_DIR}/simd_instructions.txt"

    echo "SIMD instruction report:"
    echo ""
    echo "${OUTPUT_DIR}/simd_instructions.txt"
    echo ""

fi

# BUSQUEDA HOST GPU
if [ "$IMPLEMENTATION" == "gpu" ]; then

    echo "========================================================="
    echo "SEARCHING FOR OPENCL HOST CALLS"
    echo "========================================================="
    echo ""

    grep -i "clGetPlatformIDs\|clGetDeviceIDs\|clCreateContext\|clCreateBuffer\|clEnqueueWriteBuffer\|clEnqueueNDRangeKernel\|clEnqueueReadBuffer\|clRelease" \
    "${OUTPUT_DIR}/full_disassembly.txt" \
    > "${OUTPUT_DIR}/opencl_host_calls.txt"

    echo "OpenCL host call report:"
    echo ""
    echo "${OUTPUT_DIR}/opencl_host_calls.txt"
    echo ""

    echo "Note:"
    echo "The GPU kernel assembly is not shown by objdump because kernels/fir.cl"
    echo "is compiled by the OpenCL driver at runtime."
    echo ""

fi

# FINAL
echo "========================================================="
echo "ASSEMBLY ANALYSIS COMPLETED"
echo "========================================================="
echo ""
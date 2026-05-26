#!/bin/bash

# FIR ASSEMBLY ANALYSIS SCRIPT

# USAGE:
# ./run_asm.sh scalar
# ./run_asm.sh simd

# VALIDACION ARGUMENTOS
if [ $# -lt 1 ]; then
    echo "Usage: ./run_asm.sh <implementation>"
    echo ""
    echo "Available implementations:"
    echo "  scalar"
    echo "  simd"
    exit 1
fi

IMPLEMENTATION=$1

# VALIDACION IMPLEMENTACION
if [[ "$IMPLEMENTATION" != "scalar" &&
      "$IMPLEMENTATION" != "simd" ]]; then

    echo "Invalid implementation: $IMPLEMENTATION"
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
make $IMPLEMENTATION

if [ $? -ne 0 ]; then
    echo ""
    echo "Compilation failed."
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

# EXTRAER SOLO FUNCIONES FIR
objdump -d -M intel build/fir_project | \
grep -A 120 "<fir_" \
> "${OUTPUT_DIR}/fir_functions_only.txt"

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

# FINAL
echo "========================================================="
echo "ASSEMBLY ANALYSIS COMPLETED"
echo "========================================================="
echo ""
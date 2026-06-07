#!/bin/bash

# FIR PERFORMANCE ANALYSIS SCRIPT
#
# USAGE:
#
# ./run_perf.sh scalar
# ./run_perf.sh simd
# ./run_perf.sh gpu

# VALIDACION ARGUMENTOS
if [ $# -lt 1 ]; then
    echo "Usage: ./run_perf.sh <implementation>"
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

# CONFIGURACION PERF
CORE_ID=0

PERF_EVENTS="\
cycles,\
instructions,\
cache-references,\
cache-misses,\
branches,\
branch-misses"

CACHE_EVENTS="\
L1-dcache-loads,\
L1-dcache-load-misses,\
LLC-loads,\
LLC-load-misses"

# EVENTOS SIMD/FP
SIMD_EVENTS="
fp_ret_sse_avx_ops.all,
fp_ret_sse_avx_ops.sp_add_sub_flops,
fp_ret_sse_avx_ops.sp_mult_flops,
fp_ret_sse_avx_ops.sp_mult_add_flops"

# EVENTOS EXTRA MEMORIA
MEMORY_EVENTS="\
bus-cycles"

#SELECCION EVENTOS SEGUN IMPLEMENTACION
if [ "$IMPLEMENTATION" == "scalar" ]; then
    PERF_EVENTS="${PERF_EVENTS},${CACHE_EVENTS}"
elif [ "$IMPLEMENTATION" == "simd" ]; then
    PERF_EVENTS="${PERF_EVENTS},${CACHE_EVENTS},${SIMD_EVENTS},${MEMORY_EVENTS}"
else
    PERF_EVENTS="${PERF_EVENTS}"

fi

SMALL_RUNS=1000
MEDIUM_RUNS=100
LARGE_RUNS=10

RESULTS_DIR="results/${IMPLEMENTATION}/perf"

# CREAR DIRECTORIO RESULTADOS
mkdir -p "$RESULTS_DIR"

mkdir -p "results/gpu/profiling"

# COMPILACION
echo ""
echo "========================================================="
echo "COMPILING IMPLEMENTATION: $IMPLEMENTATION"
echo "========================================================="
echo ""

make clean
make "$IMPLEMENTATION"

# VALIDAR COMPILACION
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

# FUNCION PERF
run_perf()
{
    DATASET=$1
    RUNS=$2

    OUTPUT_FILE="${RESULTS_DIR}/${DATASET}_perf.txt"

    echo ""
    echo "========================================================="
    echo "RUNNING PERF"
    echo "Implementation : $IMPLEMENTATION"
    echo "Dataset        : $DATASET"
    echo "Repetitions    : $RUNS"
    echo "Output         : $OUTPUT_FILE"
    echo "========================================================="
    echo ""

    # LIMPIEZA CACHE
    sync
    sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"

    # PERF EXECUTION
    #
    # IMPORTANTE:
    # El binario recibe SOLO:
    #
    # ./build/fir_project <dataset>
    #
    # La implementación se selecciona en compilación mediante:
    # BUILD_SCALAR, BUILD_SIMD o BUILD_GPU.
    #
    # Para GPU, perf mide principalmente el comportamiento del host:
    # dispatch OpenCL, transferencias, sincronización y lectura/escritura.
    # El trabajo interno del kernel GPU no se refleja como instrucciones CPU.
    #

    taskset -c "$CORE_ID" \
    /usr/lib/linux-tools/6.8.0-117-generic/perf stat \
    -r "$RUNS" \
    -e "$PERF_EVENTS" \
    ./build/fir_project "$DATASET" \
    2> "$OUTPUT_FILE"

    echo ""
    echo "Saved results to:"
    echo "$OUTPUT_FILE"
    echo ""
}

# EJECUCIONES
run_perf "small"  "$SMALL_RUNS"
run_perf "medium" "$MEDIUM_RUNS"
run_perf "large"  "$LARGE_RUNS"

# FINAL
echo ""
echo "========================================================="
echo "PERF ANALYSIS COMPLETED"
echo "========================================================="
echo ""
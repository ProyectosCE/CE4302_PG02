CC=gcc

# COMMON FLAGS
COMMON_CFLAGS=-O3 -Wall -Wextra -std=c11 -Iinclude

LDFLAGS=-lrt

TARGET=build/fir_project

# SCALAR FLAGS
# Scalar puro:
# - Deshabilita vectorización automática
# - Deshabilita uso automático de SIMD
SCALAR_CFLAGS=\
	$(COMMON_CFLAGS) \
	-fno-tree-vectorize \
	-mno-avx \
	-mno-avx2 \
	-mno-fma

# SIMD FLAGS
# Fuerza soporte AVX2/FMA
SIMD_CFLAGS=\
	$(COMMON_CFLAGS) \
	-mavx2 \
	-mfma

# GPU FLAGS
# Host CPU de GPU:
# - Deshabilita vectorización automática del código host.
# - Deshabilita AVX/AVX2/FMA en el host para no contaminar mediciones.
# - El kernel OpenCL se compila por el driver, no por GCC.
GPU_CFLAGS=\
	$(COMMON_CFLAGS) \
	-DBUILD_GPU \
	-fno-tree-vectorize \
	-mno-avx \
	-mno-avx2 \
	-mno-fma

GPU_LDFLAGS=\
	$(LDFLAGS) \
	-lOpenCL

# COMMON SOURCE FILES
COMMON_SRC=\
	src/main.c \
	src/dataset/dataset.c \
	src/utils/aligned_memory.c \
	src/utils/timer.c \
	src/utils/validation.c \
	src/benchmarks/benchmark_common.c

# SCALAR SOURCE FILES
SCALAR_SRC=\
	src/benchmarks/scalar_benchmark.c \
	src/scalar/fir_scalar.c

# SIMD SOURCE FILES
SIMD_SRC=\
	src/benchmarks/simd_benchmark.c \
	src/simd/fir_simd.c

# GPU SOURCE FILES
GPU_SRC=\
	src/benchmarks/gpu_benchmark.c \
	src/gpu/fir_gpu.c \
	src/gpu/opencl_utils.c

# DEFAULT TARGET
all: scalar

# SCALAR BUILD
scalar:
	mkdir -p build
	mkdir -p results/scalar/output_signal

	$(CC) \
	$(SCALAR_CFLAGS) \
	$(COMMON_SRC) \
	$(SCALAR_SRC) \
	-o $(TARGET) \
	$(LDFLAGS)

# SIMD BUILD
simd:
	mkdir -p build
	mkdir -p results/simd/output_signal

	$(CC) \
	$(SIMD_CFLAGS) \
	$(COMMON_SRC) \
	$(SIMD_SRC) \
	-o $(TARGET) \
	$(LDFLAGS)

# GPU BUILD
gpu:
	mkdir -p build
	mkdir -p results/gpu/output_signal

	$(CC) \
	$(GPU_CFLAGS) \
	$(COMMON_SRC) \
	$(GPU_SRC) \
	-o $(TARGET) \
	$(GPU_LDFLAGS)

# CLEAN
clean:
	rm -rf build/*
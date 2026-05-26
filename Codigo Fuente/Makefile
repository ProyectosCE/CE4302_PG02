CC=gcc

# COMMON FLAGS
COMMON_CFLAGS=-O3 -Wall -Wextra -std=c11 -Iinclude

LDFLAGS=-lrt -lm

TARGET=build/fir_project

# SCALAR FLAGS
SCALAR_CFLAGS=\
	$(COMMON_CFLAGS) \
	-DBUILD_SCALAR \
	-fno-tree-vectorize \
	-mno-avx \
	-mno-avx2 \
	-mno-fma

# SIMD FLAGS
SIMD_CFLAGS=\
	$(COMMON_CFLAGS) \
	-DBUILD_SIMD \
	-mavx2 \
	-mfma

# GPU FLAGS
GPU_CFLAGS=\
	$(COMMON_CFLAGS) \
	-DBUILD_GPU

GPU_LDFLAGS=\
	$(LDFLAGS) \
	-lOpenCL

# SOURCE FILES
COMMON_SRC=\
	src/main.c \
	src/dataset/dataset.c \
	src/utils/aligned_memory.c \
	src/utils/timer.c \
	src/utils/validation.c \
	src/benchmarks/benchmark.c

SCALAR_SRC=\
	src/scalar/fir_scalar.c

SIMD_SRC=\
	src/simd/fir_simd.c

GPU_SRC=\
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
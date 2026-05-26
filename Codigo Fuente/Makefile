CC=gcc

# COMMON FLAGS
COMMON_CFLAGS=-O3 -Wall -Wextra -std=c11 -Iinclude

LDFLAGS=-lrt

TARGET=build/fir_project

# SCALAR FLAGS
# Scalar puro:
# - Deshabilita vectorización automática
# - Deshabilita uso automático de SIMD
# - Mantiene optimizaciones generales de O3
SCALAR_CFLAGS=\
	$(COMMON_CFLAGS) \
	-fno-tree-vectorize \
	-mno-avx \
	-mno-avx2 \
	-mno-fma \

# SIMD FLAGS
# Fuerza soporte AVX2/FMA
# Preparado para intrinsics AVX2
SIMD_CFLAGS=\
	$(COMMON_CFLAGS) \
	-mavx2 \
	-mfma

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

# CLEAN
clean:
	rm -rf build/*
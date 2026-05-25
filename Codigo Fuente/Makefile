CC=gcc

CFLAGS=-O3 -Wall -Wextra -std=c11 -mavx2 -mfma -Iinclude

LDFLAGS=-lrt

TARGET=build/fir_project

SRC=src/main.c \
	src/dataset/dataset.c \
	src/utils/aligned_memory.c \
	src/utils/timer.c \
	src/utils/validation.c \
	src/scalar/fir_scalar.c \
	src/benchmarks/benchmark.c

all:
	mkdir -p build
	mkdir -p results/scalar/output_signal
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf build/*
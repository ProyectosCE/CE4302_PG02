CC = gcc

CFLAGS = -O3 -Wall -Wextra -std=c11 \
          -mavx2 -mfma

INCLUDES = -Iinclude

LDFLAGS = -lrt

SRC = \
	src/main.c \
	src/dataset/dataset.c \
	src/utils/aligned_memory.c \
	src/utils/timer.c \
	src/utils/validation.c \
	src/benchmarks/benchmark.c

TARGET = build/fir_project

all:
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET) $(LDFLAGS)

debug:
	mkdir -p build
	$(CC) -g -O0 -Wall $(INCLUDES) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf build/*
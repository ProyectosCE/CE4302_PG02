# Descripción general

Este proyecto implementa un sistema de filtrado FIR mediante:

- implementación secuencial (scalar),
- paralelismo SIMD usando AVX2/FMA,
- paralelismo masivo GPU usando OpenCL.

El objetivo principal es evaluar desempeño, throughput y escalabilidad sobre señales de gran tamaño utilizando técnicas de paralelismo a nivel de datos (DLP).

---

# Estructura del proyecto

```text
build/          -> binarios compilados
datasets/       -> datasets binarios y configuraciones
include/        -> headers globales
kernels/        -> kernels OpenCL
results/        -> outputs y resultados de benchmarks
scripts/        -> generación de datasets en MATLAB
src/            -> código fuente principal
```

---

# Descripción de módulos

## include/

Contiene los headers compartidos del proyecto.

* `common.h`

  * estructuras globales y constantes compartidas.

* `dataset.h`

  * interfaz de carga de datasets.

* `aligned_memory.h`

  * manejo de memoria alineada para AVX2.

* `timer.h`

  * medición precisa de tiempos.

* `validation.h`

  * validación y almacenamiento de resultados.

* `fir_scalar.h`

  * interfaz FIR secuencial.

* `fir_simd.h`

  * interfaz FIR SIMD AVX2.

* `fir_gpu.h`

  * interfaz FIR OpenCL.

---

## src/

Implementaciones principales del proyecto.

### dataset/

* carga de:

  * `signal.bin`,
  * `filters.bin`,
  * `config.txt`.

### scalar/

* implementación FIR secuencial.

### simd/

* implementación FIR usando intrinsics AVX2/FMA.

### gpu/

* implementación OpenCL GPU y utilidades OpenCL.

### utils/

* memoria alineada,
* timers,
* validación.

### benchmarks/

* ejecución automática de benchmarks y mediciones.

---

## datasets/

Datasets utilizados para benchmarking:

* `small`
* `medium`
* `large`

Cada dataset contiene:

```text
signal.bin
filters.bin
config.txt
```

---

## kernels/

Contiene kernels OpenCL:

```text
fir.cl
```

---

## results/

Directorio de resultados generados por:

* scalar,
* simd,
* gpu,
* plots.

---

# Infraestructura implementada

La infraestructura base implementada incluye:

* carga automática de datasets,
* memoria alineada a 32 bytes,
* soporte AVX2/FMA,
* soporte OpenCL,
* benchmarking centralizado,
* temporización precisa,
* validación de resultados,
* compilación automatizada.

Esta infraestructura permite desarrollar y comparar implementaciones:

* scalar,
* SIMD,
* GPU,

sobre un entorno reproducible y orientado a HPC.

---

## Compilación

Desde el directorio `Codigo Fuente/` ejecutar:

### Compilar implementación escalar

```bash
make scalar
```

### Compilar implementación SIMD AVX2/FMA

```bash
make simd
```

### Limpiar binarios compilados

```bash
make clean
```

---

# Ejecución

El proyecto utiliza el script `run.sh` para automatizar:

* compilación,
* creación de directorios,
* ejecución del benchmark.

## Uso general

```bash
./run.sh <implementation> <dataset>
```

## Implementaciones disponibles

```text
scalar
simd
gpu
```

## Datasets disponibles

```text
small
medium
large
```

---

## Ejemplos de ejecución

### Scalar

```bash
./run.sh scalar small
./run.sh scalar medium
./run.sh scalar large
```

### SIMD

```bash
./run.sh simd small
./run.sh simd medium
./run.sh simd large
```

### GPU

```bash
./run.sh gpu small
./run.sh gpu medium
./run.sh gpu large
```

---

## Ejecución manual del binario

También es posible ejecutar directamente el ejecutable compilado:

```bash
./build/fir_project <dataset>
```

Ejemplo:

```bash
./build/fir_project small
```

---

# Perfilado de rendimiento (perf)

El proyecto incluye el script `run_perf.sh` para realizar análisis de rendimiento utilizando Linux `perf`.

El script automatiza:

* compilación,
* fijación de afinidad de CPU,
* limpieza de caché,
* ejecución repetida,
* recolección de métricas HPC,
* almacenamiento de resultados.

---

## Uso general

```bash
./run_perf.sh <implementation>
```

---

## Implementaciones soportadas

```text
scalar
simd
gpu
```

---

## Ejemplos de perfilado

### Scalar

```bash
./run_perf.sh scalar
```

### SIMD

```bash
./run_perf.sh simd
```

### GPU

```bash
./run_perf.sh gpu
```

---

## Métricas recolectadas

El análisis con `perf` incluye:

* cycles
* instructions
* cache-references
* cache-misses
* branches
* branch-misses

---

## Resultados generados

Los resultados se almacenan automáticamente en:

```text
results/<implementation>/perf/
```

Ejemplos:

```text
results/scalar/perf/
results/simd/perf/
results/gpu/perf/
```

Cada implementación genera archivos separados para:

```text
small_perf.txt
medium_perf.txt
large_perf.txt
```

---

# Requisitos

## Sistema Operativo

* Ubuntu 24.04 LTS

## Compilador

* GCC con soporte AVX2/FMA

## Dependencias

* OpenCL
* Mesa Rusticl (GPU AMD)
* MATLAB (solo para generación de datasets)

---

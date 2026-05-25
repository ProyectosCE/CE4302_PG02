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

# Compilación

Desde el directorio `Codigo Fuente/` ejecutar:

```bash
make
```

Para limpiar archivos compilados:

```bash
make clean
```

---

# Ejecución

Ejecutar:

```bash
./run.sh
```

O directamente:

```bash
./build/fir_project
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

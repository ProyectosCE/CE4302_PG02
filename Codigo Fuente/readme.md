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
kernels/        -> kernels OpenCL ejecutados en GPU
results/        -> outputs, resultados de benchmarks, perf y asm
scripts/        -> scripts auxiliares de validacion, ejecucion y generacion
src/            -> codigo fuente principal
```

Estructura interna relevante:

```
include/
    common.h
    dataset.h
    aligned_memory.h
    timer.h
    validation.h

    benchmark_common.h
    scalar_benchmark.h
    simd_benchmark.h
    gpu_benchmark.h

    fir_scalar.h
    fir_simd.h
    fir_gpu.h
    opencl_utils.h

src/
    main.c

    benchmarks/
        benchmark_common.c
        scalar_benchmark.c
        simd_benchmark.c
        gpu_benchmark.c

    scalar/
        fir_scalar.c

    simd/
        fir_simd.c

    gpu/
        fir_gpu.c
        opencl_utils.c

    dataset/
        dataset.c

    utils/
        aligned_memory.c
        timer.c
        validation.c

kernels/
    fir.cl
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

* `benchmark_common.h`

  * interfaz común para impresión de resultados de benchmark.

* `scalar_benchmark.h`

  * interfaz del benchmark de la implementación escalar.

* `simd_benchmark.h`

  * interfaz del benchmark de la implementación SIMD.

* `gpu_benchmark.h`

  * interfaz del benchmark de la implementación GPU/OpenCL.

* `opencl_utils.h`

  * utilidades auxiliares para OpenCL:
    * redondeo de tamaños de trabajo,
    * validación de errores OpenCL,
    * carga de kernels `.cl`,
    * visualización del log de compilación del kernel.

---

## src/

Implementaciones principales del proyecto.

### main.c

Contiene el punto de entrada unico del proyecto.

El binario recibe unicamente el nombre del dataset:

```bash
./build/fir_project <dataset>
```

La implementacion concreta se selecciona en tiempo de compilacion mediante macros:

```text
BUILD_SCALAR
BUILD_SIMD
BUILD_GPU
```

Esto permite mantener un solo `main.c` para scalar, SIMD y GPU.

### dataset/

* carga de:

  * `signal.bin`,
  * `filters.bin`,
  * `config.txt`.

### scalar/

* implementacion FIR secuencial.
* contiene la funcion:

```text
fir_scalar()
```

### simd/

* implementacion FIR usando intrinsics AVX2/FMA.
* contiene la funcion:

```text
fir_simd()
```

### gpu/

* implementacion FIR usando OpenCL.
* contiene:

```text
fir_gpu.c          -> ejecucion del banco FIR en GPU
opencl_utils.c     -> utilidades auxiliares OpenCL
```

La implementacion GPU utiliza:

* deteccion de plataforma OpenCL,
* seleccion de dispositivo GPU,
* creacion de contexto,
* creacion de command queue,
* buffers en memoria del dispositivo,
* compilacion dinamica de `kernels/fir.cl`,
* configuracion de argumentos del kernel,
* ejecucion mediante NDRange 2D,
* medicion de tiempo puro del kernel con eventos OpenCL,
* copia de resultados GPU -> CPU.

### utils/

* memoria alineada,
* timers,
* validacion,
* construccion de rutas de salida,
* almacenamiento binario de senales filtradas.

### benchmarks/

Contiene benchmarks separados por tipo de implementacion:

```text
benchmark_common.c   -> impresion comun de resultados
scalar_benchmark.c   -> benchmark scalar
simd_benchmark.c     -> benchmark SIMD
gpu_benchmark.c      -> benchmark GPU/OpenCL
```

Cada benchmark se encarga de:

* ejecutar la implementacion correspondiente,
* medir tiempo total,
* calcular throughput,
* guardar outputs binarios,
* llenar la estructura `benchmark_result_t`.


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

# Arquitectura de la implementacion GPU/OpenCL

La implementacion GPU utiliza OpenCL para ejecutar el banco de filtros FIR sobre un dispositivo grafico compatible.

A diferencia de scalar y SIMD, donde el calculo se ejecuta directamente en CPU, la version GPU requiere una etapa adicional de preparacion del entorno OpenCL. Esta etapa es obligatoria porque la CPU y la GPU tienen espacios de ejecucion y memoria separados.

---

## Componentes OpenCL utilizados

### Plataforma OpenCL

La plataforma representa la implementacion OpenCL disponible en el sistema, por ejemplo Mesa, AMD, Intel o NVIDIA.

Se obtiene mediante:

```c
clGetPlatformIDs()
```

Este paso es necesario porque un sistema puede tener mas de una plataforma OpenCL instalada.

---

### Dispositivo OpenCL

El dispositivo representa el hardware donde se ejecutara el kernel. En este proyecto se solicita explicitamente un dispositivo GPU:

```c
clGetDeviceIDs(..., CL_DEVICE_TYPE_GPU, ...)
```

Este paso permite seleccionar el hardware de computo paralelo masivo.

---

### Contexto OpenCL

El contexto agrupa los recursos asociados al dispositivo OpenCL:

* buffers,
* programas,
* kernels,
* command queues.

Se crea mediante:

```c
clCreateContext()
```

Sin un contexto no es posible reservar memoria en el dispositivo ni compilar kernels para la GPU.

---

### Command Queue

La command queue es la cola mediante la cual el host envia comandos a la GPU.

Se utiliza para:

* copiar datos CPU -> GPU,
* lanzar kernels,
* esperar eventos,
* copiar resultados GPU -> CPU.

En este proyecto se crea con profiling habilitado:

```c
CL_QUEUE_PROFILING_ENABLE
```

Esto permite medir el tiempo puro de ejecucion del kernel mediante eventos OpenCL.

---

### Buffers OpenCL

Los buffers representan memoria reservada en el dispositivo GPU.

La implementacion utiliza tres buffers principales:

```text
signal_mem   -> senal de entrada
filters_mem  -> banco de filtros FIR
output_mem   -> resultados filtrados
```

Estos buffers son necesarios porque el kernel OpenCL no trabaja directamente con los punteros de memoria del host.

---

### Programa OpenCL

El programa OpenCL se crea a partir del archivo fuente:

```text
kernels/fir.cl
```

El archivo se carga desde disco y luego se compila en tiempo de ejecucion para el dispositivo seleccionado.

Esto permite que el driver OpenCL genere codigo compatible con la GPU disponible.

---

### Kernel OpenCL

El kernel ejecutado es:

```c
fir_filter_bank
```

Este kernel calcula el banco de filtros FIR en paralelo.

Cada work-item calcula una muestra de salida para un filtro especifico.

---

### Evento OpenCL

Se utiliza un `cl_event` asociado a la ejecucion del kernel para medir:

* inicio del kernel,
* fin del kernel,
* tiempo puro de ejecucion en GPU.

El tiempo se obtiene con:

```c
clGetEventProfilingInfo()
```

Esto permite separar:

* tiempo puro del kernel GPU,
* tiempo total de ejecucion incluyendo transferencias y overhead del host.

---

## Organizacion del kernel GPU

El kernel utiliza un NDRange bidimensional:

```text
Dimension 0 -> muestras de la senal
Dimension 1 -> filtros FIR
```

Por lo tanto:

```text
get_global_id(0) -> muestra n
get_global_id(1) -> filtro f
```

Cada work-item calcula:

```text
output[f * signal_size + n]
```

El banco de filtros se almacena de forma aplanada:

```text
filters[f * filter_order + k]
```

La salida tambien se almacena de forma aplanada:

```text
output[f * signal_size + n]
```

---

## Padding del NDRange

La dimension de muestras puede redondearse hacia arriba para que sea multiplo del tamano local del work-group.

Esto permite lanzar work-groups completos.

Los work-items adicionales se descartan dentro del kernel con:

```c
if (n >= signal_size || f >= filter_count)
{
  return;
}
```

---

## Tamano local del work-group

La implementacion usa:

```c
#define LOCAL_SIZE_X 64
```

Este valor busca alinearse con la organizacion tipica de GPUs AMD, donde una wavefront suele agrupar 64 threads.

---

## Diferencia entre tiempo de kernel y tiempo total GPU

La implementacion GPU reporta dos metricas importantes:

```text
Kernel Time
Execution Time
```

### Kernel Time

Corresponde unicamente al tiempo de ejecucion del kernel en la GPU.

### Execution Time

Incluye el flujo completo desde el host:

* creacion/preparacion OpenCL,
* transferencia CPU -> GPU,
* ejecucion del kernel,
* transferencia GPU -> CPU,
* almacenamiento de resultados.

Esta separacion es importante porque en datasets pequenos el overhead puede ser significativo, mientras que en datasets grandes el costo se amortiza mejor.

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

### Compilar implementacion GPU OpenCL

```bash
make gpu
```

La compilacion GPU enlaza contra OpenCL:

```bash
-lOpenCL
```

### Limpiar binarios compilados

```bash
make clean
```

---

## Flags de compilacion

El Makefile separa las banderas por implementacion.

### Scalar

La implementacion scalar deshabilita vectorizacion automatica y uso de AVX/FMA:

```bash
-fno-tree-vectorize
-mno-avx
-mno-avx2
-mno-fma
```

Esto permite medir una version secuencial pura.

### SIMD

La implementacion SIMD habilita AVX2/FMA explicitamente:

```bash
-mavx2
-mfma
```

Tambien puede deshabilitar vectorizacion automatica para evitar que GCC introduzca optimizaciones no controladas fuera de los intrinsics manuales:

```bash
-fno-tree-vectorize
```

### GPU

La implementacion GPU deshabilita vectorizacion automatica en el codigo host:

```bash
-fno-tree-vectorize
-mno-avx
-mno-avx2
-mno-fma
```

Esto afecta unicamente al codigo C ejecutado por CPU. El kernel OpenCL se compila por el driver OpenCL en tiempo de ejecucion.

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

# Salida esperada de GPU

Al ejecutar:

```bash
./run.sh gpu small
```

se espera una salida similar a:

```text
GPU Details
-----------
Kernel Time       : <tiempo> ms
Kernel Throughput : <muestras/segundo> samples/sec

Benchmark Result
----------------
Dataset        : small
Implementation : GPU
Execution Time : <tiempo> ms
Throughput     : <muestras/segundo> samples/sec
```

`Kernel Time` representa unicamente el tiempo puro del kernel OpenCL.

`Execution Time` representa el tiempo total medido desde el benchmark, incluyendo preparacion, transferencias y recuperacion de resultados.

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

# Validacion de resultados

El proyecto genera archivos binarios de salida por implementacion, dataset y filtro.

El formato de salida es:

```text
results/<implementation>/output_signal/<dataset>_filter_<id>.bin
```

Ejemplo:

```text
results/scalar/output_signal/small_filter_00.bin
results/simd/output_signal/small_filter_00.bin
results/gpu/output_signal/small_filter_00.bin
```

---

## Comparacion de implementaciones

El script:

```text
scripts/compare_outputs.py
```

compara las salidas de:

```text
simd
gpu
```

contra la referencia:

```text
scalar
```

La comparacion calcula:

* error absoluto maximo,
* error absoluto medio.

---

## Uso

Antes de ejecutar la comparacion, deben generarse los outputs de las tres implementaciones:

```bash
./run.sh scalar small
./run.sh simd small
./run.sh gpu small

./run.sh scalar medium
./run.sh simd medium
./run.sh gpu medium

./run.sh scalar large
./run.sh simd large
./run.sh gpu large
```

Luego ejecutar:

```bash
python3 scripts/compare_outputs.py
```

---

## Interpretacion esperada

### SIMD vs scalar

Es normal observar diferencias pequenas por punto flotante debido al uso de operaciones vectoriales y posibles cambios en el orden de acumulacion.

Errores del orden de `10^-6` se consideran esperados para `float32`.

### GPU vs scalar

La GPU puede coincidir exactamente con scalar si el kernel mantiene el mismo orden de acumulacion que la implementacion secuencial.

En cualquier caso, la validacion debe considerar tolerancias razonables para datos `float32`.

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

# Análisis de instrucciones ensamblador

El proyecto incluye el script `run_asm.sh` para generar y analizar el código ensamblador producido por GCC.

Este análisis permite verificar:

* uso correcto de instrucciones SIMD AVX2/FMA,
* ausencia de vectorización automática en scalar,
* generación real de instrucciones vectoriales,
* utilización de registros `ymm` y operaciones FMA.

---

## Uso general

```bash
./run_asm.sh <implementation>
```

---

## Implementaciones soportadas

```text
scalar
simd
gpu
```

---

## Ejemplos de análisis ensamblador

### Scalar

```bash
./run_asm.sh scalar
```

### SIMD

```bash
./run_asm.sh simd
```

### GPU

```bash
./run_asm.sh gpu
```

---

## Resultados generados

Los resultados se almacenan automáticamente en:

```text
results/<implementation>/asm_output/
```

Ejemplos:

```text
results/scalar/asm_output/
results/simd/asm_output/
results/gpu/asm_output/
```

---

## Archivos generados

Cada análisis genera:

```text
full_disassembly.txt
fir_functions_only.txt
```

### full_disassembly.txt

Contiene el desensamblado completo del binario compilado.

### fir_functions_only.txt

Contiene únicamente las funciones FIR relevantes para análisis arquitectónico.

---

## Reporte SIMD automático

Para la implementación SIMD también se genera:

```text
simd_instructions.txt
```

Este archivo contiene únicamente instrucciones vectoriales relevantes detectadas automáticamente mediante búsqueda de:

* `ymm`
* `xmm`
* `vfmadd`
* `vmovups`
* `vmulps`
* `vaddps`

---

## Nota importante sobre GPU

Para la implementacion GPU, `objdump` analiza el binario host generado por GCC.

Esto permite observar funciones como:

```text
fir_gpu
opencl_load_kernel_source
opencl_check_error
opencl_print_build_log
run_fir_gpu_benchmark
```

Sin embargo, el codigo interno del kernel OpenCL:

```text
kernels/fir.cl
```

no aparece en el desensamblado de `objdump`, porque el kernel se compila en tiempo de ejecucion por el driver OpenCL.

Por esta razon, el analisis ASM de GPU sirve para estudiar el lado host/OpenCL, pero no el ISA interno ejecutado por la GPU.

---

## Verificación esperada

### Scalar

La implementación scalar NO debe contener:

* instrucciones `ymm`,
* instrucciones AVX2,
* operaciones FMA.

Esto valida correctamente:

```bash
-fno-tree-vectorize
-mno-avx
-mno-avx2
-mno-fma
```

---

### SIMD

La implementación SIMD debe contener instrucciones como:

```asm
vfmadd231ps
vmovups
vmulps
vaddps
```

y registros vectoriales:

```asm
ymm0
ymm1
ymm2
```

Esto confirma:

* vectorización explícita,
* uso de AVX2,
* uso de FMA,
* paralelismo SIMD real.


---

# Requisitos

## Sistema Operativo

* Ubuntu 24.04 LTS
* Debian/Ubuntu compatible

## Compilador

* GCC con soporte AVX2/FMA

## Dependencias

* OpenCL
* ICD loader OpenCL
* Mesa OpenCL ICD
* clinfo
* MATLAB u Octave, solo para generacion de datasets si aplica

---

## Instalacion de OpenCL en Ubuntu/Debian

Para habilitar compilacion y ejecucion OpenCL en Ubuntu/Debian, se utilizaron los siguientes comandos:

```bash
sudo apt update
sudo apt install mesa-opencl-icd ocl-icd-opencl-dev clinfo
```

### Paquetes instalados

* `mesa-opencl-icd`

  * provee una implementacion OpenCL mediante Mesa.

* `ocl-icd-opencl-dev`

  * instala headers y librerias de desarrollo necesarias para compilar con OpenCL.

* `clinfo`

  * permite verificar plataformas y dispositivos OpenCL disponibles.

---

## Verificacion de OpenCL

Despues de instalar las dependencias, verificar la disponibilidad de OpenCL con:

```bash
clinfo
```

Tambien puede filtrarse la salida:

```bash
clinfo | grep -i "platform"
clinfo | grep -i "device"
```

El sistema debe mostrar al menos una plataforma OpenCL y un dispositivo compatible.

---

## Compilacion con OpenCL

La implementacion GPU requiere enlazar contra OpenCL:

```bash
-lOpenCL
```

El target `gpu` del Makefile realiza este enlace automaticamente.

---

## Consideraciones para GPU

Si `clinfo` no muestra un dispositivo GPU, la implementacion OpenCL puede fallar en:

```text
clGetDeviceIDs
```

porque el codigo solicita explicitamente:

```c
CL_DEVICE_TYPE_GPU
```

En ese caso, se debe revisar la instalacion del runtime OpenCL, el driver grafico o la disponibilidad real del dispositivo GPU.

#define CL_TARGET_OPENCL_VERSION 300

#include "../../include/fir_gpu.h"
#include "../../include/opencl_utils.h"
#include "../../include/timer.h"

#include <CL/cl.h>

#include <stdio.h>
#include <stdlib.h>

#define LOCAL_SIZE_X 64
#define FIR_GPU_KERNEL_PATH "kernels/fir.cl"

/**
 * @file fir_gpu.c
 * @brief Implementación GPU del banco de filtros FIR usando OpenCL.
 *
 * Este archivo contiene la implementación de la etapa de cómputo GPU para
 * el banco de filtros FIR.
 *
 * A diferencia de la implementación escalar y SIMD, esta versión no ejecuta
 * directamente el algoritmo sobre la CPU. En su lugar, utiliza OpenCL como
 * capa de abstracción para:
 *
 * - detectar una plataforma OpenCL disponible;
 * - seleccionar un dispositivo GPU;
 * - crear un contexto de ejecución;
 * - reservar memoria en el dispositivo;
 * - transferir datos desde CPU hacia GPU;
 * - compilar el kernel OpenCL en tiempo de ejecución;
 * - configurar argumentos del kernel;
 * - lanzar una ejecución paralela usando NDRange 2D;
 * - medir el tiempo puro del kernel mediante profiling;
 * - recuperar los resultados desde GPU hacia CPU.
 *
 * La implementación usa un NDRange bidimensional:
 *
 * - Dimensión 0: muestras de la señal.
 * - Dimensión 1: filtros del banco FIR.
 *
 * Por tanto, cada work-item calcula una salida:
 *
 * output[filter_id * signal_size + sample_id]
 *
 * Esta organización permite explotar paralelismo tanto entre muestras como
 * entre filtros.
 */

/**
 * @brief Ejecuta un banco de filtros FIR sobre GPU usando OpenCL.
 *
 * Esta función prepara el entorno OpenCL completo necesario para ejecutar
 * el kernel FIR en GPU. El host CPU se encarga de preparar datos,
 * reservar buffers, enviar comandos al dispositivo y recuperar resultados.
 *
 * OpenCL requiere varios objetos obligatorios:
 *
 * - cl_platform_id:
 *   Representa la plataforma OpenCL disponible en el sistema, por ejemplo
 *   AMD, Intel, NVIDIA o una implementación de CPU. Es el primer nivel de
 *   descubrimiento del hardware.
 *
 * - cl_device_id:
 *   Representa el dispositivo físico o lógico donde se ejecutará el kernel.
 *   En este caso se solicita explícitamente un dispositivo GPU mediante
 *   CL_DEVICE_TYPE_GPU.
 *
 * - cl_context:
 *   Representa el entorno de ejecución asociado al dispositivo. Todos los
 *   recursos OpenCL, como buffers, programas y colas, deben pertenecer a un
 *   contexto. Sin contexto no se puede reservar memoria ni compilar kernels.
 *
 * - cl_command_queue:
 *   Representa la cola de comandos enviados desde el host hacia el dispositivo.
 *   Las operaciones como escribir buffers, ejecutar kernels y leer resultados
 *   se encolan en este objeto. Se habilita CL_QUEUE_PROFILING_ENABLE para poder
 *   medir el tiempo real del kernel en la GPU.
 *
 * - cl_mem:
 *   Representa memoria reservada en el dispositivo. Los arreglos de CPU no son
 *   usados directamente por la GPU, por lo que se crean buffers para signal,
 *   filters y output.
 *
 * - cl_program:
 *   Representa el programa OpenCL creado a partir del código fuente del archivo
 *   .cl. Este programa debe compilarse para el dispositivo seleccionado.
 *
 * - cl_kernel:
 *   Representa la función específica del programa OpenCL que será ejecutada
 *   en paralelo. En este caso, fir_filter_bank.
 *
 * - cl_event:
 *   Representa un evento asociado a la ejecución del kernel. Se utiliza para
 *   sincronizar y obtener tiempos de inicio y fin del kernel directamente desde
 *   la cola OpenCL.
 *
 * @param signal Señal de entrada almacenada en memoria del host.
 * @param filters Banco de filtros FIR almacenado de forma aplanada.
 * @param output Buffer de salida en memoria del host.
 * @param signal_size Tamaño de la señal de entrada.
 * @param filter_order Orden de cada filtro FIR.
 * @param filter_count Cantidad de filtros FIR del banco.
 * @param kernel_time_ms Tiempo puro de ejecución del kernel GPU en milisegundos.
 *
 * @return int
 * - 0 si la ejecución fue exitosa.
 * - -1 si ocurrió un error durante la preparación, ejecución o lectura.
 */
int fir_gpu(
    const float* signal,
    const float* filters,
    float* output,
    size_t signal_size,
    size_t filter_order,
    size_t filter_count,
    double* kernel_time_ms
)
{
    /**
     * @brief Validación de punteros recibidos.
     *
     * La GPU necesita datos válidos para ejecutar el kernel. Si alguno de los
     * buffers del host es NULL, no es posible transferir datos ni almacenar
     * resultados.
     */
    if (!signal || !filters || !output || !kernel_time_ms)
    {
        fprintf(stderr,
                "Error: invalid argument passed to fir_gpu.\n");

        return -1;
    }

    /**
     * @brief Validación de dimensiones del problema FIR.
     *
     * El kernel requiere al menos una muestra, un coeficiente y un filtro.
     * Dimensiones en cero producirían lanzamientos inválidos o buffers vacíos.
     */
    if (signal_size == 0 || filter_order == 0 || filter_count == 0)
    {
        fprintf(stderr,
                "Error: invalid FIR GPU dimensions.\n");

        return -1;
    }

    /**
     * @brief Variable estándar para almacenar códigos de error OpenCL.
     *
     * La mayoría de funciones OpenCL devuelven un código de error. Este valor
     * permite verificar si cada paso fue exitoso antes de continuar.
     */
    cl_int error = CL_SUCCESS;

    /**
     * @section OpenCL Hardware Discovery
     *
     * @brief Objetos usados para descubrir y seleccionar hardware OpenCL.
     *
     * La plataforma representa la implementación OpenCL instalada en el sistema.
     * El dispositivo representa el hardware concreto que ejecutará el kernel,
     * en este caso una GPU.
     */
    cl_platform_id platform = NULL;
    cl_device_id device = NULL;

    /**
     * @section OpenCL Execution Objects
     *
     * @brief Objetos principales del entorno de ejecución OpenCL.
     *
     * El contexto agrupa los recursos asociados al dispositivo.
     * La cola permite enviar comandos al dispositivo.
     * El programa representa el código OpenCL compilado.
     * El kernel representa la función OpenCL ejecutable.
     */
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;

    /**
     * @section OpenCL Device Memory
     *
     * @brief Buffers reservados en memoria del dispositivo GPU.
     *
     * signal_mem almacena la señal de entrada.
     * filters_mem almacena los coeficientes del banco FIR.
     * output_mem almacena los resultados generados por la GPU.
     */
    cl_mem signal_mem = NULL;
    cl_mem filters_mem = NULL;
    cl_mem output_mem = NULL;

    /**
     * @brief Evento asociado a la ejecución del kernel.
     *
     * Se utiliza para esperar la finalización del kernel y obtener métricas
     * precisas de profiling directamente desde OpenCL.
     */
    cl_event kernel_event = NULL;

    cl_event write_signal_event = NULL;
    cl_event write_filters_event = NULL;
    cl_event read_output_event = NULL;

    /**
     * @brief Código fuente del kernel cargado desde kernels/fir.cl.
     *
     * OpenCL permite compilar kernels en tiempo de ejecución. Esto permite que
     * el driver genere código específico para el dispositivo seleccionado.
     */
    char* kernel_source = NULL;

    /**
     * @brief Estado final de la función.
     *
     * Se inicializa como error y solo cambia a 0 si todo el flujo OpenCL termina
     * correctamente.
     */
    int status = -1;

    FILE* profiling_file = NULL;

    profiling_file = fopen(
        "results/gpu/profiling/gpu_profiling.txt",
        "a"
    );

    if (!profiling_file)
    {
        fprintf(stderr,
                "Warning: could not open GPU profiling file.\n");
    }

    /**
     * @brief Tamaño total del buffer de salida.
     *
     * Como la salida contiene una señal filtrada por cada filtro, el total de
     * resultados es:
     *
     * signal_size * filter_count
     */
    size_t output_size = signal_size * filter_count;

    /**
     * @section NDRange Configuration
     *
     * @brief Configuración del espacio de ejecución paralelo.
     *
     * OpenCL ejecuta kernels mediante un espacio de índices llamado NDRange.
     * En esta implementación se usa un NDRange 2D:
     *
     * - global_size[0]: cantidad de work-items para muestras.
     * - global_size[1]: cantidad de work-items para filtros.
     *
     * La dimensión de muestras se redondea a múltiplo de LOCAL_SIZE_X para que
     * los work-groups queden completos. Los work-items sobrantes se descartan
     * dentro del kernel mediante una condición de borde.
     */
    size_t global_size[2] =
    {
        opencl_round_up(signal_size, LOCAL_SIZE_X),
        filter_count
    };

    /**
     * @brief Tamaño local del work-group.
     *
     * Se usa 64 work-items en la dimensión de muestras. Este valor es adecuado
     * para GPUs AMD, donde una wavefront suele agrupar 64 threads.
     *
     * La segunda dimensión local se mantiene en 1 para que cada work-group
     * procese muestras de un filtro específico.
     */
    size_t local_size[2] =
    {
        LOCAL_SIZE_X,
        1
    };

    /**
     * @section Platform Selection
     *
     * @brief Obtiene la primera plataforma OpenCL disponible.
     *
     * Este paso es obligatorio porque OpenCL puede tener múltiples plataformas
     * instaladas. Por ejemplo, una plataforma AMD, una Intel o una implementación
     * de CPU. La plataforma permite consultar dispositivos compatibles.
     */
    error = clGetPlatformIDs(
        1,
        &platform,
        NULL
    );

    if (opencl_check_error(error, "clGetPlatformIDs") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Device Selection
     *
     * @brief Selecciona un dispositivo GPU dentro de la plataforma.
     *
     * Se usa CL_DEVICE_TYPE_GPU porque esta implementación busca ejecutar el
     * banco FIR sobre una GPU. Si no existe una GPU OpenCL disponible, este paso
     * falla y la función termina.
     */
    error = clGetDeviceIDs(
        platform,
        CL_DEVICE_TYPE_GPU,
        1,
        &device,
        NULL
    );

    if (opencl_check_error(error, "clGetDeviceIDs") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Context Creation
     *
     * @brief Crea el contexto OpenCL asociado al dispositivo seleccionado.
     *
     * El contexto es obligatorio porque define el entorno donde viven los
     * recursos OpenCL. Los buffers, programas y colas deben crearse dentro de
     * un contexto.
     */
    context = clCreateContext(
        NULL,
        1,
        &device,
        NULL,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateContext") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Command Queue Creation
     *
     * @brief Configura propiedades de la cola de comandos.
     *
     * CL_QUEUE_PROFILING_ENABLE habilita medición temporal de comandos OpenCL.
     * Sin esta propiedad, clGetEventProfilingInfo no podría devolver tiempos
     * válidos del kernel.
     */
    cl_command_queue_properties queue_properties[] =
    {
        CL_QUEUE_PROPERTIES,
        CL_QUEUE_PROFILING_ENABLE,
        0
    };

    /**
     * @brief Crea la cola de comandos para enviar trabajo a la GPU.
     *
     * La cola es el mecanismo mediante el cual el host ordena operaciones:
     *
     * - copiar datos hacia la GPU;
     * - ejecutar kernels;
     * - copiar resultados hacia la CPU;
     * - sincronizar eventos.
     */
    queue = clCreateCommandQueueWithProperties(
        context,
        device,
        queue_properties,
        &error
    );

    if (opencl_check_error(error, "clCreateCommandQueueWithProperties") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Device Buffer Allocation
     *
     * @brief Reserva memoria para la señal en la GPU.
     *
     * CL_MEM_READ_ONLY indica que el kernel solo leerá este buffer.
     */
    signal_mem = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        sizeof(float) * signal_size,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateBuffer signal") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Reserva memoria para los filtros en la GPU.
     *
     * El banco de filtros se almacena aplanado. Su tamaño total es:
     *
     * filter_count * filter_order
     */
    filters_mem = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        sizeof(float) * filter_count * filter_order,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateBuffer filters") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Reserva memoria para la salida en la GPU.
     *
     * CL_MEM_WRITE_ONLY indica que el kernel escribirá los resultados en este
     * buffer. Luego el host leerá su contenido hacia output.
     */
    output_mem = clCreateBuffer(
        context,
        CL_MEM_WRITE_ONLY,
        sizeof(float) * output_size,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateBuffer output") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Host To Device Transfer
     *
     * @brief Copia la señal desde memoria del host hacia memoria de la GPU.
     *
     * clEnqueueWriteBuffer encola una transferencia CPU -> GPU.
     * Se usa CL_TRUE para que la llamada sea bloqueante y garantice que la copia
     * finalice antes de continuar.
     */
    error = clEnqueueWriteBuffer(
        queue,
        signal_mem,
        CL_TRUE,
        0,
        sizeof(float) * signal_size,
        signal,
        0,
        NULL,
        &write_signal_event
    );

    if (opencl_check_error(error, "clEnqueueWriteBuffer signal") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Copia el banco de filtros desde CPU hacia GPU.
     *
     * Esta transferencia es necesaria porque el kernel OpenCL accede a memoria
     * del dispositivo, no directamente a los punteros del host.
     */
    error = clEnqueueWriteBuffer(
        queue,
        filters_mem,
        CL_TRUE,
        0,
        sizeof(float) * filter_count * filter_order,
        filters,
        0,
        NULL,
        &write_filters_event
    );

    if (opencl_check_error(error, "clEnqueueWriteBuffer filters") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Kernel Source Loading
     *
     * @brief Carga el archivo fuente del kernel OpenCL.
     *
     * El kernel está separado en kernels/fir.cl para mantener el código de GPU
     * independiente del host C. OpenCL compila este código en tiempo de ejecución.
     */
    kernel_source = opencl_load_kernel_source(FIR_GPU_KERNEL_PATH);

    if (!kernel_source)
    {
        goto cleanup;
    }

    /**
     * @section Program Creation
     *
     * @brief Crea un programa OpenCL a partir del código fuente cargado.
     *
     * Este paso solo construye el objeto programa. Todavía no genera código
     * ejecutable para el dispositivo.
     */
    program = clCreateProgramWithSource(
        context,
        1,
        (const char**)&kernel_source,
        NULL,
        &error
    );

    if (opencl_check_error(error, "clCreateProgramWithSource") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Program Build
     *
     * @brief Compila el programa OpenCL para el dispositivo seleccionado.
     *
     * Este paso es obligatorio porque OpenCL recibe código fuente del kernel y
     * debe traducirlo a una representación ejecutable por la GPU específica.
     *
     * Si hay errores de sintaxis, incompatibilidades o problemas de compilación,
     * se imprime el log generado por el driver.
     */
    error = clBuildProgram(
        program,
        1,
        &device,
        NULL,
        NULL,
        NULL
    );

    if (error != CL_SUCCESS)
    {
        fprintf(stderr,
                "Error: OpenCL kernel build failed.\n");

        opencl_print_build_log(program, device);

        goto cleanup;
    }

    /**
     * @section Kernel Creation
     *
     * @brief Obtiene el kernel fir_filter_bank desde el programa compilado.
     *
     * El nombre debe coincidir exactamente con la función declarada en
     * kernels/fir.cl:
     *
     * __kernel void fir_filter_bank(...)
     */
    kernel = clCreateKernel(
        program,
        "fir_filter_bank",
        &error
    );

    if (opencl_check_error(error, "clCreateKernel fir_filter_bank") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Conversión de tamaños a int para coincidir con la firma del kernel.
     *
     * El kernel OpenCL recibe signal_size, filter_order y filter_count como int.
     */
    int signal_size_arg = (int)signal_size;
    int filter_order_arg = (int)filter_order;
    int filter_count_arg = (int)filter_count;

    /**
     * @section Kernel Argument Binding
     *
     * @brief Asocia los buffers y parámetros del host con los argumentos
     * definidos en el kernel OpenCL.
     *
     * El orden de clSetKernelArg debe coincidir exactamente con la firma del
     * kernel fir_filter_bank en kernels/fir.cl.
     */
    error = clSetKernelArg(
        kernel,
        0,
        sizeof(cl_mem),
        &signal_mem
    );

    if (opencl_check_error(error, "clSetKernelArg signal") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Asocia el banco de filtros al argumento 1 del kernel.
     */
    error = clSetKernelArg(
        kernel,
        1,
        sizeof(cl_mem),
        &filters_mem
    );

    if (opencl_check_error(error, "clSetKernelArg filters") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Asocia el buffer de salida al argumento 2 del kernel.
     */
    error = clSetKernelArg(
        kernel,
        2,
        sizeof(cl_mem),
        &output_mem
    );

    if (opencl_check_error(error, "clSetKernelArg output") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Asocia el tamaño de la señal al argumento 3 del kernel.
     */
    error = clSetKernelArg(
        kernel,
        3,
        sizeof(int),
        &signal_size_arg
    );

    if (opencl_check_error(error, "clSetKernelArg signal_size") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Asocia el orden del filtro al argumento 4 del kernel.
     */
    error = clSetKernelArg(
        kernel,
        4,
        sizeof(int),
        &filter_order_arg
    );

    if (opencl_check_error(error, "clSetKernelArg filter_order") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Asocia la cantidad de filtros al argumento 5 del kernel.
     */
    error = clSetKernelArg(
        kernel,
        5,
        sizeof(int),
        &filter_count_arg
    );

    if (opencl_check_error(error, "clSetKernelArg filter_count") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Kernel Launch
     *
     * @brief Encola la ejecución paralela del kernel FIR.
     *
     * clEnqueueNDRangeKernel lanza el kernel sobre el espacio NDRange definido
     * por global_size y local_size.
     *
     * Cada work-item ejecutará fir_filter_bank y calculará una posición de
     * salida asociada a una muestra n y un filtro f.
     */
    error = clEnqueueNDRangeKernel(
        queue,
        kernel,
        2,
        NULL,
        global_size,
        local_size,
        0,
        NULL,
        &kernel_event
    );

    if (opencl_check_error(error, "clEnqueueNDRangeKernel") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Espera a que el kernel termine.
     *
     * Aunque los comandos OpenCL se encolan de forma asíncrona, para medir el
     * tiempo y leer resultados se necesita garantizar que el kernel ya terminó.
     */
    error = clWaitForEvents(
        1,
        &kernel_event
    );

    if (opencl_check_error(error, "clWaitForEvents") != 0)
    {
        goto cleanup;
    }

    /**
     * @section Kernel Profiling
     *
     * @brief Obtiene timestamps de inicio y fin del kernel.
     *
     * Estos tiempos vienen del sistema de profiling de OpenCL, no de un timer
     * externo de CPU. Por eso representan mejor el tiempo puro de ejecución del
     * kernel en la cola del dispositivo.
     */
    cl_ulong kernel_start = 0;
    cl_ulong kernel_end = 0;

    error = clGetEventProfilingInfo(
        kernel_event,
        CL_PROFILING_COMMAND_START,
        sizeof(kernel_start),
        &kernel_start,
        NULL
    );

    if (opencl_check_error(error, "clGetEventProfilingInfo START") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Obtiene el timestamp de finalización del kernel.
     */
    error = clGetEventProfilingInfo(
        kernel_event,
        CL_PROFILING_COMMAND_END,
        sizeof(kernel_end),
        &kernel_end,
        NULL
    );

    if (opencl_check_error(error, "clGetEventProfilingInfo END") != 0)
    {
        goto cleanup;
    }

    /**
     * @brief Convierte el tiempo de nanosegundos a milisegundos.
     *
     * OpenCL reporta tiempos de profiling en nanosegundos.
     */
    *kernel_time_ms = (double)(kernel_end - kernel_start) / 1000000.0;

    /**
     * @section Device To Host Transfer
     *
     * @brief Copia los resultados desde GPU hacia memoria del host.
     *
     * Después de ejecutar el kernel, los resultados viven en output_mem dentro
     * de la memoria del dispositivo. Para guardarlos o validarlos desde CPU,
     * se deben copiar al buffer output del host.
     */
    double read_start_ms = get_time_ms();
    
    error = clEnqueueReadBuffer(
        queue,
        output_mem,
        CL_TRUE,
        0,
        sizeof(float) * output_size,
        output,
        0,
        NULL,
        &read_output_event
    );

    if (opencl_check_error(error, "clEnqueueReadBuffer output") != 0)
    {
        goto cleanup;
    }

    error = clWaitForEvents(
        1,
        &read_output_event
    );

    if (opencl_check_error(error, "clWaitForEvents read_output") != 0)
    {
        goto cleanup;
    }

    double read_end_ms = get_time_ms();

    double write_signal_ms = opencl_get_event_time_ms(write_signal_event);

    double write_filters_ms = opencl_get_event_time_ms(write_filters_event);

    double read_output_ms = read_end_ms - read_start_ms;

    double total_transfer_ms = write_signal_ms + write_filters_ms + read_output_ms;

    double total_gpu_pipeline_ms = total_transfer_ms + *kernel_time_ms;

    /**
     * @brief Marca la ejecución como exitosa.
     */
    status = 0;

    if (profiling_file)
    {
        fprintf(profiling_file,
                "=========== GPU PROFILING ===========\n");

        fprintf(profiling_file,
                "Signal size         : %zu\n",
                signal_size);

        fprintf(profiling_file,
                "Filter order        : %zu\n",
                filter_order);

        fprintf(profiling_file,
                "Filter count        : %zu\n",
                filter_count);

        fprintf(profiling_file,
                "H2D signal transfer : %.3f ms\n",
                write_signal_ms);

        fprintf(profiling_file,
                "H2D filters transfer: %.3f ms\n",
                write_filters_ms);

        fprintf(profiling_file,
                "Kernel execution    : %.3f ms\n",
                *kernel_time_ms);

        fprintf(profiling_file,
                "D2H output transfer : %.3f ms\n",
                read_output_ms);

        fprintf(profiling_file,
                "Total GPU pipeline  : %.3f ms\n",
                total_gpu_pipeline_ms);

        fprintf(profiling_file,
                "=====================================\n\n");

        fflush(profiling_file);
    }

cleanup:

    /**
     * @section Resource Cleanup
     *
     * @brief Libera recursos OpenCL y memoria dinámica.
     *
     * OpenCL utiliza objetos con conteo de referencias. Todo objeto creado con
     * funciones como clCreateBuffer, clCreateKernel o clCreateContext debe ser
     * liberado explícitamente para evitar fugas de memoria en el host o en el
     * dispositivo.
     *
     * El bloque cleanup se ejecuta tanto en éxito como en error, garantizando
     * una salida controlada.
     */

    if (profiling_file)
    {
        fclose(profiling_file);
    }
     if (write_signal_event)
    {
        clReleaseEvent(write_signal_event);
    }

    if (write_filters_event)
    {
        clReleaseEvent(write_filters_event);
    }

    if (read_output_event)
    {
        clReleaseEvent(read_output_event);
    }
    if (kernel_event)
    {
        clReleaseEvent(kernel_event);
    }

    if (kernel)
    {
        clReleaseKernel(kernel);
    }

    if (program)
    {
        clReleaseProgram(program);
    }

    if (signal_mem)
    {
        clReleaseMemObject(signal_mem);
    }

    if (filters_mem)
    {
        clReleaseMemObject(filters_mem);
    }

    if (output_mem)
    {
        clReleaseMemObject(output_mem);
    }

    if (queue)
    {
        clReleaseCommandQueue(queue);
    }

    if (context)
    {
        clReleaseContext(context);
    }

    /**
     * @brief Libera el código fuente del kernel cargado desde archivo.
     *
     * Este buffer fue reservado por opencl_load_kernel_source().
     */
    free(kernel_source);

    return status;
}
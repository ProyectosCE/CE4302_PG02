#define CL_TARGET_OPENCL_VERSION 300
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/cl.h>
#include "ocl_utils.h"

// --- CONFIGURACIÓN DE TU CARGA N ---
#define DATA_SIZE 1000005  // Tu 'N' real (No es múltiplo de nada)
#define LOCAL_SIZE 64

int main() {
    // 1. Calcular dinámicamente el tamaño global para la GPU (Múltiplo de 64 inmediatamente superior)
    size_t global_item_size = ((DATA_SIZE + LOCAL_SIZE - 1) / LOCAL_SIZE) * LOCAL_SIZE;
    size_t num_groups = global_item_size / LOCAL_SIZE;

    printf("Tamaño real de datos (N): %d\n", DATA_SIZE);
    printf("Tamaño global asignado a la GPU (Múltiplo de 64): %zu\n", global_item_size);
    printf("Hilos fantasmas de relleno (Padding): %zu\n", global_item_size - DATA_SIZE);
    printf("Cantidad total de Work-Groups: %zu\n\n", num_groups);

    struct timespec cpu_start, cpu_end;
    clock_gettime(CLOCK_MONOTONIC, &cpu_start);

    // Los arreglos del Host se dimensionan con el tamaño real de tus datos N
    float *A = (float*)malloc(sizeof(float) * DATA_SIZE);
    float *B = (float*)malloc(sizeof(float) * DATA_SIZE);
    float *C = (float*)malloc(sizeof(float) * DATA_SIZE);
    int *WG_IDs_host = (int*)malloc(sizeof(int) * DATA_SIZE);
    
    for(int i = 0; i < DATA_SIZE; i++) {
        A[i] = (float)i;
        B[i] = (float)(i * 2);
    }

    cl_platform_id platform_id;
    cl_device_id device_id;
    clGetPlatformIDs(1, &platform_id, NULL);
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, NULL);
    cl_command_queue_properties props[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, props, NULL);

    // Los buffers de OpenCL se crean con el tamaño real DATA_SIZE para optimizar VRAM
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, DATA_SIZE * sizeof(float), NULL, NULL);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, DATA_SIZE * sizeof(float), NULL, NULL);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, DATA_SIZE * sizeof(float), NULL, NULL);
    cl_mem wg_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, DATA_SIZE * sizeof(int), NULL, NULL);

    clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, DATA_SIZE * sizeof(float), A, 0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, DATA_SIZE * sizeof(float), B, 0, NULL, NULL);

    char *kernelSource = load_kernel_source("kernel.cl");
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, NULL);
    clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "vector_add", NULL);
    free(kernelSource);

    int n_val = DATA_SIZE;
    // Pasar argumentos incluyendo la variable de control N (Argumento 4)
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&wg_mem_obj);
    clSetKernelArg(kernel, 4, sizeof(int), (void *)&n_val);

    size_t local_item_size = LOCAL_SIZE; 
    cl_event event;

    // Se despacha usando global_item_size (que sí es múltiplo de 64)
    clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, &event);
    clWaitForEvents(1, &event);

    cl_ulong time_start, time_end;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    double gpu_duration_ms = (double)(time_end - time_start) / 1000000.0;

    clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, DATA_SIZE * sizeof(float), C, 0, NULL, NULL);
    clEnqueueReadBuffer(command_queue, wg_mem_obj, CL_TRUE, 0, DATA_SIZE * sizeof(int), WG_IDs_host, 0, NULL, NULL);

    clock_gettime(CLOCK_MONOTONIC, &cpu_end);
    double cpu_duration_ms = (cpu_end.tv_sec - cpu_start.tv_sec) * 1000.0 + (cpu_end.tv_nsec - cpu_start.tv_nsec) / 1000000.0;

    // Contabilizar la carga real procesada por cada grupo mapeado
    int *counts = (int*)calloc(num_groups, sizeof(int));
    for(int i = 0; i < DATA_SIZE; i++) {
        counts[WG_IDs_host[i]]++;
    }

    printf("==================================================\n");
    printf("DISTRIBUCIÓN DE CARGA DETECTADA EN LA GPU:\n");
    printf("==================================================\n");
    
    int cores_reales = 7; 
    size_t grupos_por_core = num_groups / cores_reales;
    
    for(int core = 0; core < cores_reales; core++) {
        int carga_acumulada = 0;
        size_t limite = (core == cores_reales - 1) ? num_groups : (core + 1) * grupos_por_core;
        for(size_t g = core * grupos_por_core; g < limite; g++) {
            carga_acumulada += counts[g];
        }
        printf("Carga recibida por Unidad de Ejecución (Core Físico) %d: %d hilos útiles\n", core, carga_acumulada);
    }
    printf("==================================================\n");
    printf("Tiempo total medido por CPU: %0.4f ms\n", cpu_duration_ms);
    printf("Tiempo de ejecución puro en GPU: %0.4f ms\n", gpu_duration_ms);
    printf("==================================================\n");

    free(counts); free(WG_IDs_host);
    clReleaseEvent(event); clReleaseKernel(kernel); clReleaseProgram(program);
    clReleaseMemObject(a_mem_obj); clReleaseMemObject(b_mem_obj);
    clReleaseMemObject(c_mem_obj); clReleaseMemObject(wg_mem_obj);
    clReleaseCommandQueue(command_queue); clReleaseContext(context);
    free(A); free(B); free(C);
    return 0;
}
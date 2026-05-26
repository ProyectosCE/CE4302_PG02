#define CL_TARGET_OPENCL_VERSION 300 // Define la versión objetivo para evitar el pragma warning
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define VECTOR_SIZE 1024

// --- CORRECCIÓN: Definición del código fuente del Kernel ---
// Este código corre directamente dentro de las Compute Units de tu Radeon
const char *kernelSource = 
"__kernel void vector_add(__global const float *A, __global const float *B, __global float *C) { \n"
"    int id = get_global_id(0);                                                                \n"
"    C[id] = A[id] + B[id];                                                                    \n"
"}                                                                                             \n";

int main() {
    // 1. Inicializar datos en la CPU (Host)
    float *A = (float*)malloc(sizeof(float) * VECTOR_SIZE);
    float *B = (float*)malloc(sizeof(float) * VECTOR_SIZE);
    float *C = (float*)malloc(sizeof(float) * VECTOR_SIZE);
    
    for(int i = 0; i < VECTOR_SIZE; i++) {
        A[i] = (float)i;
        B[i] = (float)(i * 2);
    }

    // 2. Obtener plataformas y dispositivos (GPU)
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    
    clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    // Solicitamos explícitamente un dispositivo tipo GPU
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);

    // 3. Crear Contexto y Cola de Comandos
    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, NULL);
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, NULL, NULL);

    // 4. Crear Buffers de memoria en la GPU (Device)
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, VECTOR_SIZE * sizeof(float), NULL, NULL);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, VECTOR_SIZE * sizeof(float), NULL, NULL);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, VECTOR_SIZE * sizeof(float), NULL, NULL);

    // Copiar los datos de la CPU a la GPU
    clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), A, 0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), B, 0, NULL, NULL);

    // 5. Crear y compilar el programa del Kernel en tiempo de ejecución
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, NULL);
    clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "vector_add", NULL);

    // Pasar los argumentos (punteros de memoria de la GPU) al kernel
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);

    // 6. Ejecutar el Kernel
    size_t global_item_size = VECTOR_SIZE; // 1024 hilos en total
    size_t local_item_size = 64;           // Divididos en grupos de 64 hilos (Tu GPU prefiere múltiplos de 64)
    
    clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    // Esperar a que la GPU termine
    clFinish(command_queue);

    // 7. Leer los resultados de vuelta a la CPU
    clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), C, 0, NULL, NULL);

    // Mostrar algunos resultados para verificar
    printf("Resultado verificado: A[10] + B[10] = %f + %f = %f\n", A[10], B[10], C[10]);

    // Limpieza de recursos
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(a_mem_obj);
    clReleaseMemObject(b_mem_obj);
    clReleaseMemObject(c_mem_obj);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
    
    free(A); free(B); free(C);
    return 0;
}
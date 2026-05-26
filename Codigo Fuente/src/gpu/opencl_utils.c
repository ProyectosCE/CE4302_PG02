#define CL_TARGET_OPENCL_VERSION 300

#include "../../include/opencl_utils.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @file opencl_utils.c
 * @brief Implementación de utilidades auxiliares para OpenCL.
 *
 * Este archivo agrupa funciones comunes utilizadas por la implementación GPU.
 * Su propósito es evitar que fir_gpu.c mezcle la lógica del algoritmo FIR con
 * tareas auxiliares de infraestructura OpenCL.
 *
 * Las utilidades incluidas permiten:
 *
 * - redondear tamaños de trabajo para ajustarlos al tamaño local del work-group;
 * - verificar códigos de error retornados por funciones OpenCL;
 * - cargar el código fuente de un kernel .cl desde disco;
 * - imprimir el log de compilación generado por el driver OpenCL.
 *
 * Estas funciones no ejecutan el filtro FIR directamente. Su responsabilidad es
 * apoyar la preparación, diagnóstico y ejecución segura del kernel OpenCL.
 */

/**
 * @brief Redondea un valor hacia arriba al múltiplo indicado.
 *
 * OpenCL ejecuta kernels usando un espacio de trabajo global dividido en
 * work-groups. En muchos casos conviene que el tamaño global sea múltiplo del
 * tamaño local del work-group para mantener grupos completos.
 *
 * Por ejemplo, si se tienen 100005 muestras y un tamaño local de 64, el tamaño
 * global se redondea al siguiente múltiplo de 64. Los work-items sobrantes se
 * descartan dentro del kernel mediante una validación de límites.
 *
 * @param value Valor original.
 * @param multiple Múltiplo deseado.
 *
 * @return size_t Valor redondeado hacia arriba al múltiplo más cercano.
 */
size_t opencl_round_up(
    size_t value,
    size_t multiple
)
{
    return ((value + multiple - 1) / multiple) * multiple;
}

/**
 * @brief Verifica un código de error retornado por OpenCL.
 *
 * La mayoría de funciones OpenCL retornan un código de tipo cl_int.
 * CL_SUCCESS indica que la operación fue exitosa. Cualquier otro valor
 * representa un error, por ejemplo:
 *
 * - dispositivo no encontrado;
 * - contexto inválido;
 * - memoria insuficiente;
 * - kernel inválido;
 * - argumentos mal configurados;
 * - fallo de compilación o ejecución.
 *
 * Esta función centraliza la verificación de errores para evitar repetir la
 * misma lógica después de cada llamada OpenCL.
 *
 * @param error Código de error retornado por una función OpenCL.
 * @param message Mensaje contextual que indica dónde ocurrió el error.
 *
 * @return int
 * - 0 si error es CL_SUCCESS.
 * - -1 si ocurrió un error.
 */
int opencl_check_error(
    cl_int error,
    const char* message
)
{
    if (error != CL_SUCCESS)
    {
        fprintf(stderr,
                "OpenCL error at %s: %d\n",
                message,
                error);

        return -1;
    }

    return 0;
}

/**
 * @brief Carga el código fuente de un kernel OpenCL desde un archivo.
 *
 * OpenCL permite compilar kernels en tiempo de ejecución. Para ello, el host
 * necesita leer el archivo .cl como una cadena de caracteres y pasarlo a
 * clCreateProgramWithSource().
 *
 * En este proyecto, el kernel FIR se encuentra en:
 *
 * kernels/fir.cl
 *
 * Separar el kernel en un archivo .cl tiene varias ventajas:
 *
 * - mantiene separado el código host C del código ejecutado en GPU;
 * - permite modificar el kernel sin mezclarlo con la lógica del host;
 * - facilita documentar el lado device de OpenCL;
 * - permite que el driver compile el código para el dispositivo seleccionado.
 *
 * La función abre el archivo en modo binario, calcula su tamaño, reserva memoria,
 * lee el contenido completo y agrega el carácter nulo final '\0' para que el
 * resultado sea una cadena válida de C.
 *
 * La memoria retornada debe ser liberada por quien llama usando free().
 *
 * @param filename Ruta del archivo .cl que contiene el kernel OpenCL.
 *
 * @return char*
 * - Puntero al código fuente cargado si la lectura fue exitosa.
 * - NULL si ocurrió un error.
 */
char* opencl_load_kernel_source(
    const char* filename
)
{
    /**
     * @brief Apertura del archivo del kernel.
     *
     * Se usa modo "rb" para leer el archivo como bytes sin transformaciones.
     */
    FILE* file = fopen(filename, "rb");

    if (!file)
    {
        fprintf(stderr,
                "Error: could not open OpenCL kernel file: %s\n",
                filename);

        return NULL;
    }

    /**
     * @brief Mueve el cursor al final del archivo.
     *
     * Esto permite calcular el tamaño total del archivo usando ftell().
     */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(stderr,
                "Error: could not seek OpenCL kernel file: %s\n",
                filename);

        fclose(file);

        return NULL;
    }

    /**
     * @brief Obtiene el tamaño del archivo en bytes.
     *
     * Si ftell() falla, retorna un valor negativo.
     */
    long file_size = ftell(file);

    if (file_size < 0)
    {
        fprintf(stderr,
                "Error: could not get OpenCL kernel file size: %s\n",
                filename);

        fclose(file);

        return NULL;
    }

    /**
     * @brief Regresa el cursor al inicio para poder leer el contenido.
     */
    rewind(file);

    /**
     * @brief Reserva memoria para el código fuente del kernel.
     *
     * Se reserva un byte adicional para agregar '\0' al final.
     */
    char* source = (char*)malloc((size_t)file_size + 1);

    if (!source)
    {
        fprintf(stderr,
                "Error: could not allocate memory for OpenCL kernel source.\n");

        fclose(file);

        return NULL;
    }

    /**
     * @brief Lee el contenido completo del archivo.
     *
     * Se verifica que la cantidad de bytes leídos coincida con el tamaño
     * esperado del archivo.
     */
    size_t bytes_read = fread(
        source,
        1,
        (size_t)file_size,
        file
    );

    fclose(file);

    if (bytes_read != (size_t)file_size)
    {
        fprintf(stderr,
                "Error: incomplete read of OpenCL kernel file: %s\n",
                filename);

        free(source);

        return NULL;
    }

    /**
     * @brief Convierte el contenido leído en una cadena C válida.
     *
     * clCreateProgramWithSource() recibe el código fuente del kernel como
     * cadena de caracteres.
     */
    source[file_size] = '\0';

    return source;
}

/**
 * @brief Imprime el log de compilación de un programa OpenCL.
 *
 * Cuando OpenCL compila un kernel mediante clBuildProgram(), el driver puede
 * generar mensajes de diagnóstico. Estos mensajes pueden incluir:
 *
 * - errores de sintaxis en el archivo .cl;
 * - uso de funciones no soportadas por el dispositivo;
 * - advertencias de compilación;
 * - información sobre optimizaciones realizadas por el compilador del driver.
 *
 * Este log es especialmente importante porque el kernel OpenCL se compila en
 * tiempo de ejecución, no durante la compilación normal con gcc. Por eso, un
 * error en kernels/fir.cl puede no aparecer al ejecutar make, sino hasta que
 * el programa intenta construir el kernel.
 *
 * @param program Programa OpenCL creado con clCreateProgramWithSource().
 * @param device Dispositivo para el cual se compiló el programa.
 */
void opencl_print_build_log(
    cl_program program,
    cl_device_id device
)
{
    /**
     * @brief Consulta el tamaño del log de compilación.
     *
     * Primero se llama con tamaño cero para obtener cuántos bytes se necesitan
     * reservar.
     */
    size_t log_size = 0;

    clGetProgramBuildInfo(
        program,
        device,
        CL_PROGRAM_BUILD_LOG,
        0,
        NULL,
        &log_size
    );

    /**
     * @brief Si no hay log, no hay nada que imprimir.
     */
    if (log_size == 0)
    {
        return;
    }

    /**
     * @brief Reserva memoria para almacenar el log.
     *
     * Se reserva un byte adicional para cerrar la cadena con '\0'.
     */
    char* log = (char*)malloc(log_size + 1);

    if (!log)
    {
        return;
    }

    /**
     * @brief Obtiene el contenido completo del log de compilación.
     */
    clGetProgramBuildInfo(
        program,
        device,
        CL_PROGRAM_BUILD_LOG,
        log_size,
        log,
        NULL
    );

    /**
     * @brief Asegura que el log sea una cadena C válida.
     */
    log[log_size] = '\0';

    fprintf(stderr,
            "OpenCL build log:\n%s\n",
            log);

    free(log);
}
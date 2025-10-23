// Activar features POSIX Se requiere para usar sigaction()
#define _POSIX_C_SOURCE 200809L // 200809L = POSIX.1-2008 standard

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

/* Variable global para controlar el bucle principal */
/* 'volatile' asegura que el compilador no optimice el acceso a esta variable */
/* 'sig_atomic_t' garantiza que su acceso sea seguro dentro de un manejador de señales */
volatile sig_atomic_t g_seguir_ejecutando = 1;

/**
 * @brief Manejador de la señal SIGTERM : Se ejecuta cuando el proceso recibe SIGTERM.
 *  Para terminar el bucle principal, se establece la variable g_seguir_ejecutando a 0
 * @param signum Numero de la señal recibida
 */
void manejador_sigterm(int signum) {
    /* Usamos write() porque es seguro en manejadores de señales*/
    const char mensaje[] = "\n[SEÑAL] SIGTERM recibida. Terminando proceso de forma ordenada...\n";
    write(STDOUT_FILENO, mensaje, sizeof(mensaje) - 1);
    g_seguir_ejecutando = 0;
}

/**
 * @brief Manejador de la señal SIGINT : Indica al usuario que SIGINT esta siendo ignorado.
 * @param signum Numero de la señal recibida
 */
void manejador_sigint(int signum) {
    const char mensaje[] = "\n[SEÑAL] SIGINT (Ctrl+C) recibida pero IGNORADA. Usa SIGTERM para terminar.\n";
    write(STDOUT_FILENO, mensaje, sizeof(mensaje) - 1);
}

/**
 * @brief Configura los manejadores de señales usando sigaction()
 * * sigaction() es mas robusto que signal() porque:
 * - Comportamiento consistente entre sistemas UNIX
 * - Control mas preciso sobre el comportamiento de las señales
 * - Evita condiciones de carrera
 * * @return 0 en exito, -1 en error
 */
int configurar_manejadores_senales() {
    struct sigaction accion_term, accion_int;

    /* Configurar manejador para SIGTERM */
    // Limpiar la estructura, inicializando los valores a 0. Usamos memset (memoria set) para hacerlo
    memset(&accion_term, 0, sizeof(accion_term));
    //Asiganmos la funcion que manejara la señal de terminacion (SIGTERM)
    accion_term.sa_handler = manejador_sigterm;
    // Le enviamos "sa_mask" a 0 para que no bloquee ninguna otra señal
    sigemptyset(&accion_term.sa_mask);     
    // Configuramos sa_flag en 0 para que no tenga banderas especiales
    accion_term.sa_flags = 0;

    // registramos la señal de terminacion (SIGTERM) con la funcion manejadora `manejador_sigterm` que fue configurada en la signaction "accion_term"
    // Es decir cuando llegua la señal SIGTERM, se llama la función `manejador_sigterm`
    if (sigaction(SIGTERM, &accion_term, NULL) == -1) {
        perror("Error al configurar SIGTERM");
        return -1;
    }

    /* Configurar manejador para SIGINT */
    // Se realiza el mismo proceso de configuracion, pero para la señal de interrupcion (SIGINT) con la funcion `manejador_sigint`
    memset(&accion_int, 0, sizeof(accion_int));
    accion_int.sa_handler = manejador_sigint;  
    sigemptyset(&accion_int.sa_mask);          
    accion_int.sa_flags = 0;                   

    if (sigaction(SIGINT, &accion_int, NULL) == -1) {
        perror("Error al configurar SIGINT");
        return -1;
    }

    return 0;
}
/*
Para ejecutar el programa:
    -> gcc -o signals signals.c; ./signals
*/
int main() {
    int contador = 0;

    printf("********** EJERCICIO 01: Manejo de Señales SIGINT y SIGTERM **********\n");

    if (configurar_manejadores_senales() == -1) {
        fprintf(stderr, "Error al configurar manejadores de señales\n");
        exit(EXIT_FAILURE);
    }

    printf("Configuracion de señales:\n");
    printf("- SIGINT (Ctrl+C): IGNORADA\n");
    printf("- SIGTERM: CAPTURADA (terminara el proceso)\n\n");
    printf("Instrucciones:\n");
    printf("1. Presiona Ctrl+C para probar SIGINT (sera ignorado)\n");
    printf("2. Ejecuta 'kill -SIGTERM %d' desde otra terminal para terminar\n\n", getpid());

    printf("Proceso Iniciado -> Ejecutando bucle...\n");
    printf("----------------------------------------\n");

    /* Bucle principal */
    while (g_seguir_ejecutando) {
        printf("Iteracion [%d]  Proceso activo... (PID: %d)\n", ++contador, getpid());
        sleep(2);
    }

    printf("\n----------------------------------------\n");
    printf("Proceso Terminado ->  Despues de %d iteraciones.\n", contador);

    return EXIT_SUCCESS;
}
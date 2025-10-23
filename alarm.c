// Activar features POSIX Se requiere para usar sigaction()
#define _POSIX_C_SOURCE 200809L // 200809L = POSIX.1-2008 standard

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

/* Variables globales para el control del programa */
/* 'volatile sig_atomic_t' variables globales modificadas en manejadores */
volatile sig_atomic_t g_alarma_disparada = 0;
volatile sig_atomic_t g_interrumpido = 0;

/* 'static' limita el alcance de estas globales a este archivo */
static unsigned int g_segundos_alarma = 10; //Valor por defecto en 10 segundos
static time_t g_tiempo_inicio;

/**
 * @brief Manejador de la señal SIGALRM: Se ejecuta cuando expira el temporizador establecido con alarm().
 * SIGALRM se genera automaticamente despues del tiempo especificado.
 */
void manejador_sigalrm(int signum) {
    const char mensaje[] = "\n[ALARMA] ¡Tiempo agotado! SIGALRM recibida.\n";
    write(STDOUT_FILENO, mensaje, sizeof(mensaje) - 1);
    g_alarma_disparada = 1;
}

/**
 * @brief Manejador de la señal SIGINT : Termina el proceso antes de que expire la alarma.
 */
void manejador_sigint(int signum) {
    const char mensaje[] = "\n[SEÑAL] SIGINT recibida. Cancelando alarma y terminando...\n";
    write(STDOUT_FILENO, mensaje, sizeof(mensaje) - 1);
    
    /* Cancelar la alarma pendiente */
    alarm(0); 
    g_interrumpido = 1;
}

/**
 * @brief Configura los manejadores de señales
 * @return 0 en exito, -1 en error
 */
int configurar_manejadores_senales() {
    struct sigaction accion_alarma, accion_int;
    
    /* Configurar manejador para SIGALRM */
    memset(&accion_alarma, 0, sizeof(accion_alarma));
    accion_alarma.sa_handler = manejador_sigalrm;
    sigemptyset(&accion_alarma.sa_mask);
    accion_alarma.sa_flags = 0;
    
    if (sigaction(SIGALRM, &accion_alarma, NULL) == -1) {
        perror("Error al configurar SIGALRM");
        return -1;
    }
    
    /* Configurar manejador para SIGINT */
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

/**
 * @brief Calcula el tiempo restante hasta la alarma
 * @return Segundos restantes
 */
unsigned int obtener_tiempo_restante() {
    time_t tiempo_actual = time(NULL);
    time_t transcurrido = tiempo_actual - g_tiempo_inicio;
    
    if (transcurrido >= g_segundos_alarma) {
        return 0;
    }
    
    return g_segundos_alarma - transcurrido;
}

/**
 * @brief Muestra la barra de progreso del temporizador
 *
 * @param restante Segundos restantes
 */
void mostrar_progreso(unsigned int restante) {
    const int ancho_barra = 40;
    float progreso = 1.0f - ((float)restante / g_segundos_alarma);
    int relleno = (int)(progreso * ancho_barra);
    
    /* \r (retorno de carro) mueve el cursor al inicio de la linea sin saltar */
    printf("\r["); 
    for (int i = 0; i < ancho_barra; i++) {
        if (i < relleno) {
            printf("=");
        } else if (i == relleno) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    printf("] %u/%u segundos", g_segundos_alarma - restante, g_segundos_alarma);
    
    /* fflush(stdout) fuerza a que se imprima en la terminal AHORA */
    /* (Normalmente printf espera a un salto de linea para imprimir) */
    fflush(stdout);
}

/**
Para ejecutar el programa:
    -> gcc -o alarm alarm.c; ./alarm
    -> gcc -o alarm alarm.c; ./alarm 5
 */
int main(int argc, char *argv[]) {
    unsigned int restante;
    
    printf("=== EJERCICIO 2: Uso de alarm() y SIGALRM ===\n");
    printf("PID del proceso: %d\n\n", getpid());
    
    /* Procesar argumentos de linea de comandos */
    if (argc > 1) {
        int segundos_entrada = atoi(argv[1]);
        if (segundos_entrada <= 0 || segundos_entrada > 3600) {
            fprintf(stderr, "Error: Ingresa un valor entre 1 y 3600 segundos\n");
            exit(EXIT_FAILURE);
        }
        g_segundos_alarma = segundos_entrada;
    }
    
    /* Configurar manejadores de señales */
    if (configurar_manejadores_senales() == -1) {
        fprintf(stderr, "Error al configurar manejadores de señales\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Configuracion:\n");
    printf("- Temporizador: %u segundos\n", g_segundos_alarma);
    printf("- Señal SIGALRM se generara automaticamente al expirar\n");
    printf("- Presiona Ctrl+C para cancelar antes del tiempo\n\n");
    
    /* Registrar el tiempo de inicio */
    g_tiempo_inicio = time(NULL);
    
    /* Configurar la alarma */
    printf("Iniciando alarma de %u segundos...\n", g_segundos_alarma);
    alarm(g_segundos_alarma);
    
    printf("\nProceso trabajando mientras espera la alarma...\n");
    printf("----------------------------------------\n");
    
    /* Bucle principal - simula trabajo mientras espera la alarma */
    int iteracion = 0;
    while (!g_alarma_disparada && !g_interrumpido) {
        restante = obtener_tiempo_restante();
        
        /* Mostrar barra de progreso */
        mostrar_progreso(restante);
        
        /* Simular trabajo del proceso */
        sleep(1);
        iteracion++;
    }
    
    printf("\n----------------------------------------\n");
    
    if (g_alarma_disparada) {
        printf("\n[RESULTADO] El proceso termino por la alarma (SIGALRM)\n");
        printf("Tiempo total de ejecucion: %u segundos\n", g_segundos_alarma);
    } else if (g_interrumpido) {
        printf("\n[RESULTADO] El proceso fue interrumpido manualmente (SIGINT)\n");
        time_t tiempo_fin = time(NULL);
        printf("Tiempo de ejecucion: %ld segundos de %u programados\n", 
               tiempo_fin - g_tiempo_inicio, g_segundos_alarma);
    }
    
    printf("Total de iteraciones: %d\n", iteracion);
    printf("Limpieza completada. Proceso terminado exitosamente.\n");
    
    return EXIT_SUCCESS;
}
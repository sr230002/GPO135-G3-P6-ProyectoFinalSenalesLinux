# 📡 Proyecto: Señales en Linux

Proyecto educativo sobre el manejo de señales en sistemas operativos Linux utilizando el lenguaje C y el estándar POSIX.

---

## 🎯 Descripción General

Este proyecto implementa dos ejercicios prácticos que demuestran el correcto manejo de señales en Linux:

- **Ejercicio 01 | signals**: Captura y manejo de señales **SIGINT** y **SIGTERM**
- **Ejercicio 02 | alarm**: Uso de temporizadores con `alarm()` y manejo de **SIGALRM**


## 📚 Glosario

### ¿Qué es POSIX?

**POSIX** = **P**ortable **O**perating **S**ystem **I**nterface (for Uni**X**)

Es un estándar IEEE que define la interfaz entre programas y sistemas operativos tipo UNIX (Linux, macOS, BSD). Garantiza portabilidad del código entre diferentes sistemas.

- **POSIX.1-2008**: Portable Operating System Interface
- **C11**: Estándar ISO/IEC 9899:2011

```c
#define _POSIX_C_SOURCE 200809L  // Activa características POSIX 2008
```

### Señales en Linux

Las **señales** son notificaciones asíncronas que el kernel envía a los procesos para comunicar eventos. Cada señal tiene un número único identificador.

#### Señales Principales Comunes

| Señal | Identificador | Origen | Descripción |
|-------|--------|--------|-------------|
| **SIGINT** | 2 | Ctrl+C | Interrupción interactiva |
| **SIGTERM** | 15 | `kill <PID>` | Terminación solicitada |
| **SIGALRM** | 14 | `alarm()` | Temporizador expirado |

### Estructura `sigaction`

La estructura `sigaction` define cómo manejar una señal. Es el equivalente en C a configurar un event listener en otros lenguajes.

```c
struct sigaction {
    void (*sa_handler)(int);    // Función manejadora (simple)
    sigset_t sa_mask;          // Señales a bloquear durante ejecución
    int sa_flags;              // Banderas de configuración
};
```

#### Equivalente conceptual en Java

```java
class SignalAction {
    Function<Integer, Void> handler;  // Función a ejecutar
    Set<Integer> mask;                // Señales a bloquear
    int flags;                        // Opciones de comportamiento
}
```

#### Campo: `sa_handler`

Puntero a la función que se ejecutará cuando llegue la señal.

```c
void mi_manejador(int signum) {
    write(STDOUT_FILENO, "Señal recibida\n", 15);
}

struct sigaction sa;
sa.sa_handler = mi_manejador;  // Asignar función
```

#### Campo: `sa_mask`

Conjunto de señales que se bloquearán **temporalmente** mientras se ejecuta el manejador.

```c
sigset_t mask;

sigemptyset(&mask);              // Inicializar vacío
sigaddset(&mask, SIGINT);        // Agregar SIGINT al conjunto
sigdelset(&mask, SIGINT);        // Quitar SIGINT del conjunto
sigfillset(&mask);               // Agregar TODAS las señales
sigismember(&mask, SIGINT);      // Verificar si está en el conjunto
```

**Ejemplo:**
```c
sigemptyset(&sa.sa_mask);         // No bloquear nada
sigaddset(&sa.sa_mask, SIGINT);   // Bloquear SIGINT durante el manejador
```

#### Campo: `sa_flags`

Banderas que modifican el comportamiento del manejador.

| Bandera | Descripción |
|---------|-------------|
| `0` | Sin opciones especiales |
| `SA_RESTART` | Reintentar syscalls interrumpidas automáticamente |
| `SA_SIGINFO` | Usar `sa_sigaction` para recibir información extra |
| `SA_NODEFER` | Permitir que la señal se reciba de nuevo durante su manejo |
| `SA_RESETHAND` | Restaurar comportamiento por defecto después de manejar |

```c
sa.sa_flags = 0;                      // Por defecto
sa.sa_flags = SA_RESTART;             // Una opción
sa.sa_flags = SA_RESTART | SA_NODEFER; // Múltiples (con OR)
```

### Abreviatura "sa"

**sa** = **S**ignal **A**ction (Acción de Señal)

Es una convención de nomenclatura en POSIX para todos los campos relacionados con `sigaction`.

### `signal()` vs `sigaction()`

| Característica | `signal()` | `sigaction()` |
|----------------|------------|---------------|
| **Estándar** | ANSI C (antiguo) | POSIX (moderno) |
| **Portabilidad** | Inconsistente entre sistemas | Comportamiento garantizado |
| **Control** | Limitado | Completo |
| **Recomendación** | ❌ No usar | ✅ **USAR SIEMPRE** |

### Funciones Async-Signal-Safe

Solo ciertas funciones pueden usarse dentro de manejadores de señales:

✅ **Funciones seguras:**
- `write()` - Escribir en descriptores de archivo
- `_exit()` - Terminar proceso inmediatamente
- `signal()`, `sigaction()` - Manipular señales

❌ **Funciones NO seguras:**
- `printf()` - Usar `write()` en su lugar
- `malloc()`, `free()` - Gestión de memoria
- Funciones de I/O estándar (`fopen`, `fprintf`, etc.)

### Tipo `sig_atomic_t`

Tipo de dato que garantiza operaciones atómicas (no interrumpibles) para variables compartidas entre el manejador de señales y el programa principal.

```c
volatile sig_atomic_t keep_running = 1;

void manejador(int sig) {
    keep_running = 0;  // Escritura atómica garantizada
}

int main() {
    while (keep_running) {  // Lectura atómica garantizada
        // Trabajo...
    }
}
```

---


## 🔧 Ejercicio 1: Signals

### Descripción

Programa que demuestra el manejo diferenciado de señales:
- **SIGINT (Ctrl+C)**: Se **ignora** - el proceso continúa ejecutándose
- **SIGTERM**: Se **captura** - el proceso termina ordenadamente

### Funcionamiento del programa

1. El proceso se ejecuta en un bucle infinito
2. Muestra su PID para facilitar pruebas
3. Si presionas **Ctrl+C** → Señal ignorada (mensaje informativo)
4. Si envías **SIGTERM** desde otra terminal→ Limpieza ordenada y terminación


### Flujo de Señales

```
Terminal         Kernel Linux        signals.c
   |                  |                  |
   | Ctrl+C           |                  |
   |----------------->|                  |
   |                  | SIGINT (2)       |
   |                  |----------------->|
   |                  |                  | manejador_sigint()
   |                  |                  | "Señal ignorada"
   |                  |                  |
   | kill -TERM PID   |                  |
   |----------------->|                  |
   |                  | SIGTERM (15)     |
   |                  |----------------->|
   |                  |                  | manejador_sigterm()
   |                  |                  | Limpieza y exit(0)
```


## ⏰ Ejercicio 2: Alarm

### Descripción

Programa que implementa un temporizador usando `alarm()`:
- Configura una alarma de N segundos
- Captura **SIGALRM** para terminación automática
- Permite cancelación manual con **SIGINT (Ctrl+C)**

### Comportamiento

1. Inicia temporizador de N segundos (configurable)
2. Muestra progreso en tiempo real
3. Al expirar → **SIGALRM** termina el proceso
4. **Ctrl+C** → Cancela alarma y termina anticipadamente


### Función `alarm()`

```c
unsigned int alarm(unsigned int seconds);
```

- Programa la entrega de **SIGALRM** después de `seconds` segundos
- Solo puede haber **una alarma activa** por proceso
- `alarm(0)` **cancela** cualquier alarma pendiente
- Retorna los segundos restantes de alarmas previas (0 si no había)


## ⚙️ Compilación y Ejecución

### Requisitos

- Sistema operativo: Linux (Ubuntu, Debian, Fedora, etc.)
- Compilador: GCC (GNU Compiler Collection)
- Estándar: C11 con extensiones POSIX

### Compilación

#### Ejercicio 1: Signals.c

```bash
gcc -o signals signals.c; ./signals
```

#### Ejercicio 2: Alarm.c

```bash
gcc -o alarm alarm.c; ./alarm
```
##### Utilizando argumento de tiempo para señal ALARM - ( ./alarm {segundos} )
```bash
gcc -o alarm alarm.c; ./alarm 5
```

*Proyecto es con fines educativos para demuestrar conceptos fundamentales de programación de sistemas en Linux.*
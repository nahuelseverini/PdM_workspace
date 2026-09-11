# Práctica 3 - Programación de Microprocesadores (CESE - FIUBA)

## Datos del Autor
* **Alumno:** Nahuel Severini
* **Carrera:** Carrera de Especialización en Sistemas Embebidos (CESE)
* **Cohorte:** 27Co2026
* **Materia:** Programación de Microprocesadores

## Descripción del Proyecto

En esta práctica se encapsula la lógica de retardos no bloqueantes de la Práctica 2 dentro de un módulo independiente (`API_delay`).

El programa hace titilar de forma periódica el LED de usuario (**LD2**) con un ciclo de trabajo (*duty cycle*) del 50%, recorriendo la secuencia de tiempos `{500, 100, 100, 1000}` ms.

Al completarse cada ciclo completo (encendido + apagado), se verifica con `delayIsRunning()` que el retardo haya finalizado antes de modificar su duración con `delayWrite()`. Todas las funciones incluyen control de parámetros de entrada (punteros no nulos y duraciones mayores a cero).

---

## Consigna de la Práctica

### Objetivo:

Implementar un módulo de software para trabajar con retardos no bloqueantes a partir de las funciones creadas en la práctica 2.

### Punto 1

Crear un nuevo proyecto como copia del proyecto realizado para la práctica 2.

Crear una carpeta API dentro de la carpeta Drivers en la estructura de directorios del nuevo proyecto. Crear dentro de la carpeta API, subcarpetas /Src y /Inc.

Encapsular las funciones necesarias para usar retardos no bloqueantes en un archivo fuente API_delay.c con su correspondiente archivo de cabecera API_delay.h, y ubicar estos archivos en la carpeta API creada.

En API_delay.h se deben ubicar los prototipos de las funciones y declaraciones:

```c
typedef uint32_t tick_t; // Qué biblioteca se debe incluir para que esto compile?
typedef bool bool_t;     // Qué biblioteca se debe incluir para que esto compile?
typedef struct{
   tick_t startTime;
   tick_t duration;
   bool_t running;
} delay_t;
void delayInit( delay_t * delay, tick_t duration );
bool_t delayRead( delay_t * delay );
void delayWrite( delay_t * delay, tick_t duration );
```

En API_delay.c se deben ubicar la implementación de todas las funciones.

**Nota:** cuando se agregan carpetas a un proyecto de Eclipse se deben incluir en el include path para que se incluya su contenido en la compilación. Se debe hacer clic derecho sobre la carpeta con los archivos de encabezamiento y seleccionar la opción *add/remove include path*.

### Punto 2

Implementar un programa que utilice retardos no bloqueantes y haga titilar en forma periódica un led de la placa NUCLEO-F4xx de acuerdo a una secuencia predeterminada:

```c
const uint32_t TIEMPOS[] = {500, 100, 100, 1000};
```

Utilizar la función delayWrite y una única variable tipo delay_t para cambiar el tiempo de encendido del led.

**NOTA:** los tiempos indicados son de encendido y el led debe trabajar con duty = 50%.

### Punto 3

Implementar la siguiente función auxiliar pública en API_delay.c:

```c
bool_t delayIsRunning(delay_t * delay);
```

Esta función debe devolver una copia del valor del campo running de la estructura delay_t.

Utilizar esta función en el código implementado para el punto dos para verificar que el delay no esté corriendo antes de cambiar su valor con delayWrite.

---

## Preguntas y Conclusiones

### 1. ¿Es suficientemente clara la consigna 2 o da lugar a implementaciones con distinto comportamiento?
Da lugar a distintas interpretaciones. No explicita cuántas repeticiones debe tener cada período antes de pasar al siguiente (a diferencia de la Práctica 2 que pedía 5 veces), ni aclara el estado inicial del LED (encendido o apagado). Además, al exigir un *duty cycle* del 50%, requiere sincronizar dos eventos del retardo (ON y OFF) antes de modificar la duración, lo cual puede resolverse de múltiples formas (banderas auxiliares, lectura del pin GPIO o máquinas de estados).

### 2. ¿Se puede cambiar el tiempo de encendido del LED fácilmente en un solo lugar del código o éste está hardcodeado? ¿Hay números “mágicos” en el código?
Sí, se modifica fácilmente en un solo lugar editando el arreglo `TIEMPOS[]`. No existen números mágicos: la cantidad de elementos se calcula dinámicamente con la macro `#define CANTIDAD_TIEMPOS (sizeof(TIEMPOS) / sizeof(TIEMPOS[0]))`, lo que permite agregar o quitar tiempos sin tocar la lógica del programa.

### 3. ¿Qué bibliotecas estándar se debieron agregar a API_delay.h para que el código compile? Si las funcionalidades de una API propia crecieran, ¿cuál sería el mejor lugar para incluir esas bibliotecas y typedefs?
Se incluyeron `<stdint.h>` (para el tipo `uint32_t`) y `<stdbool.h>` (para el tipo `bool`). Si la API creciera, la buena práctica dicta:
* En el archivo de cabecera público (`.h`) incluir **únicamente** las bibliotecas y `typedef` estrictamente necesarios para los tipos que el usuario debe manipular.
* Todo tipo de dato, definición o biblioteca de uso interno debe residir exclusivamente en el archivo fuente (`.c`) o en una cabecera privada interna (ej. `API_delay_port.h` o `API_delay_private.h`) para evitar contaminar el espacio de nombres (*namespace*) del proyecto principal y acotar dependencias.

### 4. ¿Es adecuado el control de los parámetros pasados por el usuario que se hace en las funciones implementadas? ¿Se controla que sean valores válidos y dentro de los rangos esperados?
Sí, es adecuado. Se implementaron funciones auxiliares privadas (`static`) para validar que los punteros no sean nulos (`delay != NULL`) y que la duración sea estrictamente positiva (`duration > 0`).
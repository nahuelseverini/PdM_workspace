# Práctica 4 - Programación de Microprocesadores (CESE - FIUBA)

## Datos del Autor
* **Alumno:** Nahuel Severini
* **Carrera:** Carrera de Especialización en Sistemas Embebidos (CESE)
* **Cohorte:** 27Co2026
* **Materia:** Programación de Microprocesadores

## Descripción del Proyecto

El objetivo de esta práctica es implementar una **Máquina de Estados Finitos (MEF)** para resolver el filtrado de rebotes por software (*antirrebote*) del pulsador de usuario de la placa NUCLEO-F446RE, utilizando retardos no bloqueantes basados en el módulo `API_delay`.

---

## Consigna de la Práctica

### Objetivo:

Implementar un MEF para trabajar con anti-rebotes por software.

### Punto 1

Crear un nuevo proyecto como copia del proyecto realizado para la práctica 3.

Implementar una MEF anti-rebote que permita leer el estado del pulsador de la placa NUCLEO-F4 y generar acciones o eventos ante un flanco descendente o ascendente, de acuerdo al siguiente diagrama:

El estado inicial de la MEF debe ser `BUTTON_UP`.

Implementar dentro de `main.c`, las funciones:


```c
void debounceFSM_init();	// debe cargar el estado inicial
void debounceFSM_update();	// debe leer las entradas, resolver la lógica de
							// transición de estados y actualizar las salidas
void buttonPressed();		// debe encender el LED
void buttonReleased();		// debe apagar el LED 
```

El tiempo de anti-rebote debe ser de 40 ms con un retardo no bloqueante como los implementados en la práctica 3.

La función `debounceFSM_update()` debe llamarse periódicamente.

```c
typedef enum{
BUTTON_UP,
BUTTON_FALLING,
BUTTON_DOWN,
BUTTON_RAISING,
} debounceState_t;

```

### Punto 2

Implementar un módulo de software en un archivo fuente `API_debounce.c` con su correspondiente archivo de cabecera `API_debounce.h` y ubicarlos en el proyecto dentro de  las carpetas `/drivers/API/src` y `/drivers/API/inc`, respectivamente.

En `API_debounce.h` se deben ubicar los prototipos de las funciones públicas y las declaraciones:

```c
void debounceFSM_init();
void debounceFSM_update();
```

La función `readKey` debe leer una variable interna del módulo y devolver `true` o `false` si la tecla fue presionada.  Si devuelve `true`, debe resetear (poner en `false`) el estado de la variable.

```c
bool_t readKey();
```
En `API_debounce.c` se deben ubicar las declaraciones privadas, los prototipos de las funciones privadas y la implementación de todas las funciones del módulo, privadas y públicas:

La declaración de `debounceState_t` debe ser privada en el archivo .c y la variable de estado de tipo `debounceState_t `debe ser global privada (con `static`).

Declarar en `API_debounce.c` una variable tipo `bool_t` global privada que se ponga en true cuando ocurre un flanco descendente y se ponga en false cuando se llame a la función `readKey()`;

Implementar un programa que cambie la frecuencia de parpadeo del LED entre 100 ms y 500 ms cada vez que se presione la tecla.  El programa debe usar las funciones anti-rebote del módulo `API_debounce` y los retardos no bloqueantes del módulo `API_delay` y la función `readKey`.

---

## Preguntas y Conclusiones

### 1. ¿Es adecuado el control de los parámetros pasados por el usuario que se hace en las funciones implementadas? ¿Se controla que sean valores válidos? ¿Se controla que estén dentro de los rangos correctos?
En este módulo, las funciones públicas (`debounceFSM_init`, `debounceFSM_update` y `readKey`) no reciben parámetros (`void`), por lo que no hay argumentos que el usuario pueda pasar de forma errónea (como punteros nulos o números negativos). 

El control en este caso pasa por el encapsulamiento: todas las variables del módulo (`currentState`, `debounceDelay` y `keyPressed`) están declaradas con `static` dentro de `API_debounce.c`. De esta forma, el usuario no puede alterar el estado de la MEF desde afuera ni saltearse transiciones. Además, en el `switch` de la MEF se incluyó el caso `default:` que reinicia la máquina llamando a `debounceFSM_init()` si la variable de estado llegara a tomar un valor inválido. Por su parte, las funciones de `API_delay` siguen chequeando que los punteros no sean nulos y que los tiempos sean mayores a cero.

### 2. ¿Se nota una mejora en la detección de las pulsaciones respecto a la práctica 0? ¿Se pierden pulsaciones? ¿Hay falsos positivos?
Sí, la diferencia es muy clara. En la práctica 0, al leer el pin directamente sin filtrar o usando demoras bloqueantes (`HAL_Delay`), los rebotes mecánicos del pulsador generaban varias lecturas falsas en una sola pulsada (falsos positivos), o bien se perdían pulsaciones si el código estaba clavado esperando que termine un delay. 

Con la máquina de estados y el tiempo de 40 ms no bloqueante, el sistema espera a que la señal eléctrica termine de rebotar y se estabilice antes de confirmar la pulsación o la liberación. En las pruebas no se observaron falsos positivos ni pulsaciones perdidas al presionar y soltar el botón.

### 3. ¿Es adecuada la temporización con la que se llama a debounceFSM_update()? ¿Y a readKey()? ¿Qué pasaría si se llamara con un tiempo mucho más grande? ¿Y mucho más corto?
* **Temporización actual:** Al llamarlas dentro del `while(1)` sin funciones bloqueantes, el ciclo se ejecuta continuamente en el orden de los microsegundos. Esto es adecuado porque revisa la máquina de estados con mucha frecuencia y detecta el fin de los 40 ms casi en el mismo milisegundo en que se cumplen, respondiendo al instante.
* **Si se llamaran con un tiempo mucho más grande (por ejemplo, más de 40 ms o 100 ms):** El sistema se volvería lento para responder. Si el tiempo entre llamadas fuera más largo que lo que tarda una persona en apretar y soltar el botón en una pulsación rápida, el programa directamente perdería la pulsación.
* **Si se llamaran con un tiempo más corto:** Actualmente ya se llaman tan rápido como el microcontrolador puede ejecutar el bucle libre. Para esta práctica funciona muy bien, aunque en un proyecto con requerimientos de bajo consumo sería mejor llamar a `debounceFSM_update()` a intervalos fijos (por ejemplo, cada 5 ms o 10 ms usando una interrupción de timer), permitiendo que el microcontrolador entre en modo reposo (*sleep*) en los tiempos intermedios.
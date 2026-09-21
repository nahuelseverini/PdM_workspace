/*
 * API_delay.h
 */

#ifndef API_DELAY_H_
#define API_DELAY_H_

#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Tipo de dato para representar marcas de tiempo en milisegundos.
 */
typedef uint32_t tick_t;

/**
 * @brief Tipo de dato para representar valores lógicos booleanos.
 */
typedef bool bool_t;

/**
 * @brief Estructura para la gestión de retardos no bloqueantes.
 */
typedef struct {
   tick_t startTime; /**< Marca de tiempo del inicio del retardo */
   tick_t duration;  /**< Duración configurada en milisegundos */
   bool_t running;   /**< Estado del retardo (true: activo, false: inactivo) */
} delay_t;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Inicializa la estructura del retardo con una duración específica.
 * @param  delay: Puntero a la estructura de tipo delay_t a inicializar.
 * @param  duration: Duración del retardo en milisegundos.
 * @retval None
 */
void delayInit(delay_t * delay, tick_t duration);

/**
 * @brief  Consulta de forma no bloqueante si el tiempo configurado ya transcurrió.
 *         Si el retardo no estaba ejecutándose, toma la marca de tiempo inicial.
 * @param  delay: Puntero a la estructura de tipo delay_t.
 * @retval bool_t: true si el tiempo transcurrió, false en caso contrario o si hubo error.
 */
bool_t delayRead(delay_t * delay);

/**
 * @brief  Modifica la duración de un retardo previamente inicializado.
 * @param  delay: Puntero a la estructura de tipo delay_t.
 * @param  duration: Nueva duración del retardo en milisegundos.
 * @retval None
 */
void delayWrite(delay_t * delay, tick_t duration);

/**
 * @brief  Devuelve el estado de ejecución actual del retardo.
 * @param  delay: Puntero a la estructura de tipo delay_t.
 * @retval bool_t: true si el retardo está corriendo, false en caso contrario.
 */
bool_t delayIsRunning(delay_t * delay);

#endif /* API_DELAY_H_ */

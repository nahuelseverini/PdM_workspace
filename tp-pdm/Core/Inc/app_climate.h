/**
 * @file    app_climate.h
 * @brief   Control de temperatura y humedad: MEF principal y alarma.
 * @author  Nahuel Severini
 */

#ifndef APP_CLIMATE_H_
#define APP_CLIMATE_H_

#include <stdbool.h>

#define APP_CLIMATE_MIN_THRESHOLD       15.0f   /* °C */
#define APP_CLIMATE_MAX_THRESHOLD       40.0f   /* °C */
#define APP_CLIMATE_DEFAULT_THRESHOLD   28.0f   /* °C */
#define APP_CLIMATE_THRESHOLD_STEP       1.0f   /* °C que sube o baja por pulsación */

typedef enum {
    CLIMATE_STATE_INIT = 0,    /* arranque, esperando la primera lectura */
    CLIMATE_STATE_NORMAL,      /* temperatura por debajo del umbral */
    CLIMATE_STATE_CONFIG,      /* se está cambiando el umbral */
    CLIMATE_STATE_ALARM,       /* temperatura igual o mayor al umbral */
    CLIMATE_STATE_ERROR        /* el sensor no responde */
} climate_state_t;

/**
 * @brief  Inicializa el sensor y deja la MEF en INIT.
 */
void app_climate_init(void);

/**
 * @brief  Atiende el botón, lee el sensor cada 2 s, actualiza la MEF y el LED.
 *         Se llama desde el loop principal.
 */
void app_climate_update(void);

/**
 * @brief  Devuelve el estado actual de la MEF.
 */
climate_state_t app_climate_get_state(void);

/**
 * @brief  Devuelve el nombre de un estado ("NORMAL", "ALARM", etc.).
 * @param  state Estado del que se quiere el nombre.
 */
const char *app_climate_state_name(climate_state_t state);

/**
 * @brief  Devuelve el umbral de alarma en °C.
 */
float app_climate_get_threshold(void);

/**
 * @brief  Cambia el umbral y pasa a modo CONFIG.
 * @param  new_threshold Valor entre 15 y 40 °C.
 * @retval true si se cambió, false si el valor está fuera de rango.
 */
bool app_climate_set_threshold(float new_threshold);

/**
 * @brief  Suma (o resta, si es negativo) un valor al umbral y pasa a modo CONFIG.
 *         Si supera los límites (15 a 40 °C), da la vuelta al otro extremo.
 * @param  delta Valor a sumar en °C.
 */
void app_climate_adjust_threshold(float delta);

/**
 * @brief  Devuelve la última temperatura leída en °C.
 */
float app_climate_get_temperature(void);

/**
 * @brief  Devuelve la última humedad leída en %.
 */
float app_climate_get_humidity(void);

/**
 * @brief  Indica si la última lectura del sensor fue válida.
 */
bool app_climate_is_sensor_ok(void);

#endif /* APP_CLIMATE_H_ */

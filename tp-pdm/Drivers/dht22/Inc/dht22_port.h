/**
 * @file    dht22_port.h
 * @brief   Parte del DHT22 que depende del hardware (pin PB5 y tiempos).
 * @author  Nahuel Severini
 */

#ifndef DHT22_PORT_H_
#define DHT22_PORT_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Configura el pin PB5 como entrada.
 */
void dht22_port_init(void);

/**
 * @brief  Pone el pin como salida (open-drain) para enviar la señal de inicio.
 */
void dht22_port_set_output(void);

/**
 * @brief  Pone el pin como entrada para leer la respuesta del sensor.
 */
void dht22_port_set_input(void);

/**
 * @brief  Escribe un nivel en el pin.
 * @param  level true = alto, false = bajo.
 */
void dht22_port_pin_write(bool level);

/**
 * @brief  Espera en milisegundos (bloquea).
 */
void dht22_port_delay_ms(uint32_t ms);

/**
 * @brief  Espera en microsegundos (bloquea).
 */
void dht22_port_delay_us(uint32_t us);

/**
 * @brief  Espera a que el pin llegue a un nivel.
 * @param  level      Nivel esperado (true = alto, false = bajo).
 * @param  timeout_us Tiempo máximo de espera.
 * @retval true si llegó al nivel, false si se agotó el tiempo.
 */
bool dht22_port_wait_pin_level(bool level, uint32_t timeout_us);

/**
 * @brief  Mide cuánto tiempo el pin se mantiene en alto.
 * @param  timeout_us Tiempo máximo de espera.
 * @retval Duración del pulso en µs, o 0 si se agotó el tiempo.
 */
uint32_t dht22_port_measure_high_pulse_us(uint32_t timeout_us);

#endif /* DHT22_PORT_H_ */

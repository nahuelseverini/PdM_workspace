/**
 * @file    dht22.c
 * @brief   Sensor de temperatura y humedad DHT22.
 * @author  Nahuel Severini
 */

#include "dht22.h"
#include "dht22_port.h"
#include <stddef.h>

#define START_LOW_MS      2     /* tiempo en bajo para pedirle una lectura al sensor */
#define START_WAIT_US     30    /* espera antes de escuchar la respuesta */
#define TIMEOUT_US        150   /* máximo que se espera cada cambio de nivel */
#define BIT_1_MIN_US      40    /* un pulso alto más largo que esto es un 1 */
#define TOTAL_BITS        40    /* humedad (16) + temperatura (16) + checksum (8) */

void dht22_init(void)
{
    dht22_port_init();
}

bool dht22_read(dht22_data_t *data)
{
    if (data == NULL) {
        return false;
    }

    uint8_t bytes[5] = {0};

    /* Pedido de lectura: el micro baja la línea 2 ms y la suelta */
    dht22_port_set_output();
    dht22_port_pin_write(false);
    dht22_port_delay_ms(START_LOW_MS);
    dht22_port_pin_write(true);
    dht22_port_delay_us(START_WAIT_US);
    dht22_port_set_input();

    /* Respuesta del sensor: bajo, alto y bajo otra vez antes de los datos */
    if (!dht22_port_wait_pin_level(false, TIMEOUT_US) ||
        !dht22_port_wait_pin_level(true, TIMEOUT_US)  ||
        !dht22_port_wait_pin_level(false, TIMEOUT_US)) {
        return false;
    }

    /* Cada bit empieza con un pulso en alto: el ancho dice si es 0 o 1 */
    for (int i = 0; i < TOTAL_BITS; i++) {
        if (!dht22_port_wait_pin_level(true, TIMEOUT_US)) {
            return false;
        }

        uint32_t pulse_us = dht22_port_measure_high_pulse_us(TIMEOUT_US);
        if (pulse_us == 0) {
            return false;
        }

        bytes[i / 8] <<= 1;
        if (pulse_us > BIT_1_MIN_US) {
            bytes[i / 8] |= 1;
        }
    }

    /* El último byte es la suma de los otros cuatro */
    uint8_t checksum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
    if (bytes[4] != checksum) {
        return false;
    }

    uint16_t raw_humidity = ((uint16_t)bytes[0] << 8) | bytes[1];
    data->humidity = raw_humidity / 10.0f;

    /* El bit más alto de la temperatura es el signo */
    int16_t raw_temperature = ((bytes[2] & 0x7F) << 8) | bytes[3];
    if (bytes[2] & 0x80) {
        raw_temperature = -raw_temperature;
    }
    data->temperature = raw_temperature / 10.0f;

    return true;
}

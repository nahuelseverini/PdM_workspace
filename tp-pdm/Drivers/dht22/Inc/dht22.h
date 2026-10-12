/**
 * @file    dht22.h
 * @brief   Sensor de temperatura y humedad DHT22.
 * @author  Nahuel Severini
 */

#ifndef DHT22_H_
#define DHT22_H_

#include <stdbool.h>

typedef struct {
    float temperature;  /* en °C */
    float humidity;     /* en % */
} dht22_data_t;

/**
 * @brief  Prepara el pin del sensor.
 */
void dht22_init(void);

/**
 * @brief  Lee temperatura y humedad (bloquea unos 7 ms).
 * @param  data Donde se guardan los valores leídos.
 * @retval true si la lectura es válida, false si falló o el checksum no coincide.
 */
bool dht22_read(dht22_data_t *data);

#endif /* DHT22_H_ */

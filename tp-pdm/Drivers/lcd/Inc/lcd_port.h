/**
 * @file    lcd_port.h
 * @brief   Parte del LCD que depende del hardware (I2C1 en PB8/PB9 y tiempos).
 * @author  Nahuel Severini
 */

#ifndef LCD_PORT_H_
#define LCD_PORT_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Configura el I2C1 y sus pines.
 * @retval true si quedó listo.
 */
bool lcd_port_init(void);

/**
 * @brief  Envía bytes a un dispositivo I2C.
 * @param  dev_addr Dirección I2C de 7 bits.
 * @param  p_data   Bytes a enviar.
 * @param  size     Cantidad de bytes.
 * @retval true si el dispositivo respondió.
 */
bool lcd_port_write(uint8_t dev_addr, uint8_t *p_data, uint16_t size);

/**
 * @brief  Revisa si un dispositivo I2C responde.
 * @param  dev_addr Dirección I2C de 7 bits.
 * @param  trials   Cantidad de intentos.
 * @retval true si respondió.
 */
bool lcd_port_is_ready(uint8_t dev_addr, uint32_t trials);

/**
 * @brief  Espera en milisegundos (bloquea).
 */
void lcd_port_delay_ms(uint32_t ms);

/**
 * @brief  Espera en microsegundos (bloquea).
 */
void lcd_port_delay_us(uint32_t us);

#endif /* LCD_PORT_H_ */

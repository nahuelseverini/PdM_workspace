/**
 * @file    lcd.h
 * @brief   Display LCD 16x2 (HD44780) conectado por I2C.
 * @author  Nahuel Severini
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <stdbool.h>

#define LCD_COLUMNS  16
#define LCD_ROWS     2

/**
 * @brief  Inicializa el LCD. Busca el módulo en la dirección I2C 0x27 o 0x3F.
 * @retval true si lo encontró, false si no responde.
 */
bool lcd_init(void);

/**
 * @brief  Borra la pantalla.
 */
void lcd_clear(void);

/**
 * @brief  Mueve el cursor.
 * @param  row Fila (0 o 1).
 * @param  col Columna (0 a 15).
 */
void lcd_set_cursor(uint8_t row, uint8_t col);

/**
 * @brief  Escribe un texto desde la posición del cursor.
 * @param  str Texto terminado en '\0'.
 */
void lcd_send_string(const char *str);

#endif /* LCD_H_ */

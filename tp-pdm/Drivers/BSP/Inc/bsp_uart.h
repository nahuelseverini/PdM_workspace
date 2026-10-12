/**
 * @file    bsp_uart.h
 * @brief   Puerto serie USART2 (consola por el USB del ST-LINK).
 * @author  Nahuel Severini
 */

#ifndef BSP_UART_H_
#define BSP_UART_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Lee un byte recibido, sin esperar si no llegó ninguno.
 * @param  p_byte Donde se guarda el byte leído.
 * @retval true si había un byte, false si no.
 */
bool bsp_uart_receive_byte(uint8_t *p_byte);

/**
 * @brief  Envía un texto terminado en '\0' (bloquea hasta enviarlo).
 * @param  str Texto a enviar.
 */
void bsp_uart_send_string(const char *str);

#endif /* BSP_UART_H_ */

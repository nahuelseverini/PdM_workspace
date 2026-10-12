/**
 * @file    app_uart.h
 * @brief   Consola serie: comandos del usuario y envío periódico de mediciones.
 * @author  Nahuel Severini
 */

#ifndef APP_UART_H_
#define APP_UART_H_

/**
 * @brief  Prepara la consola y envía el mensaje de bienvenida.
 */
void app_uart_init(void);

/**
 * @brief  Lee los caracteres recibidos, ejecuta comandos y envía la telemetría.
 *         Se llama desde el loop principal.
 */
void app_uart_update(void);

#endif /* APP_UART_H_ */

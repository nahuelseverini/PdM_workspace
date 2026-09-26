/*
 * API_uart.h
 */

#ifndef API_UART_H_
#define API_UART_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef bool_t
typedef bool bool_t;
#define bool_t bool_t
#endif

/**
 * @brief Tamaño máximo permitido para cadenas transmitidas y recibidas por la UART.
 */
#define UART_MAX_STRING_SIZE 256

/**
 * @brief Baudrate configurado para la comunicación serie.
 */
#define UART_BAUDRATE 115200

/* Public function prototypes -----------------------------------------------*/

/**
 * @brief  Inicializa el periférico UART (USART2 a 115200 baud, 8N1) y envía por
 *         la terminal serie un mensaje con los parámetros de configuración.
 * @param  None
 * @retval bool_t: true si la inicialización fue exitosa, false en caso contrario.
 */
bool_t uartInit(void);

/**
 * @brief  Transmite por UART una cadena de caracteres terminada en '\0'.
 * @param  pstring: Puntero a la cadena de caracteres a transmitir.
 * @retval None
 */
void uartSendString(uint8_t * pstring);

/**
 * @brief  Transmite por UART una cantidad específica de caracteres.
 * @param  pstring: Puntero al buffer de datos a transmitir.
 * @param  size: Cantidad de caracteres a transmitir (entre 1 y UART_MAX_STRING_SIZE).
 * @retval None
 */
void uartSendStringSize(uint8_t * pstring, uint16_t size);

/**
 * @brief  Recibe por UART una cantidad específica de caracteres.
 * @param  pstring: Puntero al buffer donde almacenar los caracteres recibidos.
 * @param  size: Cantidad de caracteres a recibir (entre 1 y UART_MAX_STRING_SIZE).
 * @retval None
 */
void uartReceiveStringSize(uint8_t * pstring, uint16_t size);

/**
 * @brief  Recibe un único byte por UART en modo polling no bloqueante.
 * @param  pbyte: Puntero a la variable donde se almacenará el byte recibido.
 * @retval bool_t: true si se recibió un byte correctamente, false si no hay datos disponibles o error.
 */
bool_t uartReceiveByte(uint8_t * pbyte);

/**
 * @brief Baudrate mínimo admitido para reconfiguración dinámica.
 */
#define UART_MIN_BAUDRATE 9600

/**
 * @brief Baudrate máximo admitido para reconfiguración dinámica.
 */
#define UART_MAX_BAUDRATE 921600

/**
 * @brief  Obtiene la velocidad de transmisión (baudrate) actualmente configurada en la UART.
 * @param  None
 * @retval uint32_t: Baudrate actual en baudios (bps).
 */
uint32_t uartGetBaudRate(void);

/**
 * @brief  Reconfigura la velocidad de transmisión (baudrate) del periférico UART.
 *         Valida que la velocidad solicitada se encuentre dentro del rango válido
 *         [UART_MIN_BAUDRATE, UART_MAX_BAUDRATE] y reinicia el periférico USART2.
 * @param  baudRate: Nueva velocidad deseada en baudios (bps).
 * @retval bool_t: true si la reconfiguración fue exitosa, false en caso contrario.
 */
bool_t uartSetBaudRate(uint32_t baudRate);

#endif /* API_UART_H_ */

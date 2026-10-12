/**
 * @file    bsp_uart.c
 * @brief   Puerto serie USART2 (consola por el USB del ST-LINK).
 * @author  Nahuel Severini
 */

#include "bsp_uart.h"
#include "main.h"
#include <stddef.h>
#include <string.h>

#define TX_TIMEOUT_MS  100U

extern UART_HandleTypeDef huart2;

bool bsp_uart_receive_byte(uint8_t *p_byte)
{
    if (p_byte == NULL) {
        return false;
    }

    /* Si se perdió un byte por overrun, se limpia el error para seguir recibiendo */
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE)) {
        __HAL_UART_CLEAR_OREFLAG(&huart2);
    }

    /* Timeout 0: no espera si no hay nada */
    return (HAL_UART_Receive(&huart2, p_byte, 1, 0) == HAL_OK);
}

void bsp_uart_send_string(const char *str)
{
    if (str == NULL) {
        return;
    }

    uint16_t length = (uint16_t)strlen(str);
    if (length > 0) {
        HAL_UART_Transmit(&huart2, (const uint8_t *)str, length, TX_TIMEOUT_MS);
    }
}

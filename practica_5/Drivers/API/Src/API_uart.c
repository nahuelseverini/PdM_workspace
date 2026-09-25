/*
 * API_uart.c
 */

#include "API_uart.h"
#include "stm32f4xx_hal.h"

/* Private define ------------------------------------------------------------*/
/**
 * @brief Tiempo de espera límite en milisegundos para operaciones de transmisión y recepción.
 */
#define UART_TIMEOUT_MS 1000

/* Private variables ---------------------------------------------------------*/
/**
 * @brief Manejador del periférico USART2, encapsulado de forma privada al módulo.
 */
static UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief  Función interna para el manejo defensivo de errores en el módulo UART.
 * @param  None
 * @retval None
 */
static void uartErrorHandler(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Función interna para el manejo defensivo de errores en el módulo UART.
 * @param  None
 * @retval None
 */
static void uartErrorHandler(void)
{
    /* Punto de captura para diagnóstico o depuración interna */
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Inicializa el periférico UART (USART2 a 115200 baud, 8N1) y envía por
 *         la terminal serie un mensaje con los parámetros de configuración.
 * @param  None
 * @retval bool_t: true si la inicialización fue exitosa, false en caso contrario.
 */
bool_t uartInit(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = UART_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        uartErrorHandler();
        return false;
    }

    /* Envía por terminal serie los parámetros de configuración iniciales */
    uartSendString((uint8_t *)"\r\n========================================\r\n");
    uartSendString((uint8_t *)" UART inicializada con éxito:\r\n");
    uartSendString((uint8_t *)" - Periférico : USART2\r\n");
    uartSendString((uint8_t *)" - Baudrate   : 115200 bps\r\n");
    uartSendString((uint8_t *)" - Parámetros : 8 bits de datos, 1 bit de parada, sin paridad (8N1)\r\n");
    uartSendString((uint8_t *)"========================================\r\n");

    return true;
}

/**
 * @brief  Transmite por UART una cadena de caracteres terminada en '\0'.
 * @param  pstring: Puntero a la cadena de caracteres a transmitir.
 * @retval None
 */
void uartSendString(uint8_t * pstring)
{
    if (pstring == NULL)
    {
        uartErrorHandler();
        return;
    }

    uint16_t length = 0;
    while (pstring[length] != '\0')
    {
        length++;
        if (length > UART_MAX_STRING_SIZE)
        {
            uartErrorHandler();
            return;
        }
    }

    if (length < 1)
    {
        uartErrorHandler();
        return;
    }

    if (HAL_UART_Transmit(&huart2, pstring, length, UART_TIMEOUT_MS) != HAL_OK)
    {
        uartErrorHandler();
    }
}

/**
 * @brief  Transmite por UART una cantidad específica de caracteres.
 * @param  pstring: Puntero al buffer de datos a transmitir.
 * @param  size: Cantidad de caracteres a transmitir (entre 1 y UART_MAX_STRING_SIZE).
 * @retval None
 */
void uartSendStringSize(uint8_t * pstring, uint16_t size)
{
    if (pstring == NULL)
    {
        uartErrorHandler();
        return;
    }

    if (size < 1 || size > UART_MAX_STRING_SIZE)
    {
        uartErrorHandler();
        return;
    }

    if (HAL_UART_Transmit(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK)
    {
        uartErrorHandler();
    }
}

/**
 * @brief  Recibe por UART una cantidad específica de caracteres.
 * @param  pstring: Puntero al buffer donde almacenar los caracteres recibidos.
 * @param  size: Cantidad de caracteres a recibir (entre 1 y UART_MAX_STRING_SIZE).
 * @retval None
 */
void uartReceiveStringSize(uint8_t * pstring, uint16_t size)
{
    if (pstring == NULL)
    {
        uartErrorHandler();
        return;
    }

    if (size < 1 || size > UART_MAX_STRING_SIZE)
    {
        uartErrorHandler();
        return;
    }

    if (HAL_UART_Receive(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK)
    {
        uartErrorHandler();
    }
}

/**
 * @brief  Recibe un único byte por UART en modo polling no bloqueante.
 * @param  pbyte: Puntero a la variable donde se almacenará el byte recibido.
 * @retval bool_t: true si se recibió un byte correctamente, false si no hay datos disponibles o error.
 */
bool_t uartReceiveByte(uint8_t * pbyte)
{
    if (pbyte == NULL)
    {
        return false;
    }

    /* Limpia la bandera de sobreescritura (ORE) si estuviera activa para evitar bloqueos */
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE))
    {
        __HAL_UART_CLEAR_OREFLAG(&huart2);
    }

    if (HAL_UART_Receive(&huart2, pbyte, 1, 0) == HAL_OK)
    {
        return true;
    }

    return false;
}

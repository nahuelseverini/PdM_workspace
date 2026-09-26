/*
 * API_cmdparser.h
 */

#ifndef API_CMDPARSER_H_
#define API_CMDPARSER_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef bool_t
typedef bool bool_t;
#define bool_t bool_t
#endif

/* Exported constants --------------------------------------------------------*/

/**
 * @brief Longitud máxima de una línea de comando (excluyendo el terminador nulo).
 */
#define CMD_MAX_LINE 64

/**
 * @brief Cantidad máxima de tokens procesables por comando (comando + argumentos).
 */
#define CMD_MAX_TOKENS 3

/**
 * @brief Cantidad máxima de bytes procesados por invocación a cmdPoll() (no bloqueante).
 */
#define CMD_MAX_BYTES_PER_POLL 16

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Códigos de estado y error del parser de comandos.
 */
typedef enum {
    CMD_OK = 0,         /**< Comando procesado o ejecutado con éxito */
    CMD_ERR_OVERFLOW,   /**< La línea recibida superó el límite de caracteres (CMD_MAX_LINE) */
    CMD_ERR_SYNTAX,     /**< Error genérico de sintaxis */
    CMD_ERR_UNKNOWN,    /**< Comando no reconocido */
    CMD_ERR_ARG         /**< Argumentos incorrectos, faltantes o no válidos */
} cmd_status_t;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Inicializa el módulo parser de comandos.
 * @param  None
 * @retval None
 */
void cmdParserInit(void);

/**
 * @brief  Máquina de estados del parser. Debe ser llamada periódicamente desde el bucle
 *         principal. Procesa hasta 16 bytes por invocación (no bloqueante).
 * @param  None
 * @retval None
 */
void cmdPoll(void);

/**
 * @brief  Transmite por UART la lista completa de comandos soportados por el
 *         sistema junto con una breve descripción de cada uno.
 * @param  None
 * @retval None
 */
void cmdPrintHelp(void);

#endif /* API_CMDPARSER_H_ */

/*
 * API_cmdparser.c
 */

#include "API_cmdparser.h"
#include "API_uart.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

/* Private types -------------------------------------------------------------*/

/**
 * @brief Estados canónicos de la Máquina de Estados Finitos (MEF) del parser.
 *
 * Diagrama de transiciones:
 *   [CMD_IDLE] --(carácter != '\r','\n','\0')--> [CMD_RECEIVING]
 *   [CMD_RECEIVING] --(delimitador '\r','\n','\0')--> [CMD_PROCESS]
 *   [CMD_RECEIVING] --(longitud > CMD_MAX_LINE)--> [CMD_ERROR]
 *   [CMD_PROCESS] --(comando válido)--> [CMD_EXEC]
 *   [CMD_PROCESS] --(comentario o vacía)--> [CMD_IDLE]
 *   [CMD_PROCESS] --(error de sintaxis/tokens)--> [CMD_ERROR]
 *   [CMD_EXEC] --(ejecución exitosa)--> [CMD_IDLE]
 *   [CMD_EXEC] --(error de argumento/comando)--> [CMD_ERROR]
 *   [CMD_ERROR] --(emisión de mensaje de error)--> [CMD_IDLE]
 */
typedef enum {
    CMD_IDLE = 0,   /**< Estado de reposo: esperando la llegada del primer byte útil */
    CMD_RECEIVING,  /**< Estado de recepción: acumulando caracteres en el búfer de línea */
    CMD_PROCESS,    /**< Estado de procesamiento: análisis léxico, filtrado y tokenización */
    CMD_EXEC,       /**< Estado de ejecución: validación semántica y acción de comandos */
    CMD_ERROR       /**< Estado de error: reporte de fallo con fin de línea y reinicio */
} cmd_fsm_state_t;

/* Private define ------------------------------------------------------------*/

/**
 * @brief Tamaño del búfer para formateo de respuestas numéricas (ej. baudrate).
 */
#define CMD_RESP_BUFFER_SIZE 32

/**
 * @brief Prefijo de texto para comando de configuración de velocidad.
 */
#define CMD_BAUD_STR_PREFIX "BAUD="
#define CMD_BAUD_PREFIX_LEN 5
#define CMD_BAUD_BASE 10

/**
 * @brief Mensajes constantes de respuesta del protocolo serie (todos finalizados en \r\n).
 */
#define CMD_MSG_OK             "OK\r\n"
#define CMD_MSG_LED_ON         "LED is ON\r\n"
#define CMD_MSG_LED_OFF        "LED is OFF\r\n"
#define CMD_MSG_ERR_OVERFLOW   "ERROR: line too long\r\n"
#define CMD_MSG_ERR_UNKNOWN    "ERROR: unknown command\r\n"
#define CMD_MSG_ERR_ARG        "ERROR: bad arguments\r\n"
#define CMD_MSG_ERR_SYNTAX     "ERROR: syntax error\r\n"

/* Private variables ---------------------------------------------------------*/

/**
 * @brief Estado actual de la MEF del parser de comandos.
 */
static cmd_fsm_state_t fsmState = CMD_IDLE;

/**
 * @brief Búfer para almacenar la línea de texto recibida (CMD_MAX_LINE incluye '\0').
 */
static char rxLineBuffer[CMD_MAX_LINE];

/**
 * @brief Índice del próximo carácter a escribir dentro de rxLineBuffer.
 */
static uint8_t rxLineIndex = 0;

/**
 * @brief Bandera que señala la condición de desbordamiento por línea demasiado larga.
 */
static bool overflowOccurred = false;

/**
 * @brief Arreglo de punteros a los tokens identificados (comando y argumentos).
 */
static char * cmdTokens[CMD_MAX_TOKENS];

/**
 * @brief Cantidad de tokens detectados en la línea actual.
 */
static uint8_t cmdTokenCount = 0;

/**
 * @brief Código de error actual para emitir en el estado CMD_ERROR.
 */
static cmd_status_t cmdErrorStatus = CMD_OK;

/* Private function prototypes -----------------------------------------------*/

/* Abstracción del hardware de LED (desacoplamiento de actuadores) */
static void ledOn(void);
static void ledOff(void);
static void ledToggle(void);
static bool_t ledGetStatus(void);

/* Funciones auxiliares internas de la MEF */
static void cmdFsmReset(void);
static void cmdProcessLine(void);
static void cmdExecute(void);
static void cmdHandleError(void);
static void cmdToUpperCase(char * str);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Enciende el LED de usuario (LD2) abstrayendo la llamada a la HAL.
 * @param  None
 * @retval None
 */
static void ledOn(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

/**
 * @brief  Apaga el LED de usuario (LD2) abstrayendo la llamada a la HAL.
 * @param  None
 * @retval None
 */
static void ledOff(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  Conmuta (invierte) el estado actual del LED de usuario (LD2).
 * @param  None
 * @retval None
 */
static void ledToggle(void)
{
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

/**
 * @brief  Lee y retorna el estado lógico actual del pin del LED de usuario (LD2).
 * @param  None
 * @retval bool_t: true si el LED está encendido (nivel alto), false si está apagado.
 */
static bool_t ledGetStatus(void)
{
    return (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET);
}

/**
 * @brief  Reinicia las variables internas y el buffer del parser al estado inicial.
 * @param  None
 * @retval None
 */
static void cmdFsmReset(void)
{
    rxLineIndex = 0;
    rxLineBuffer[0] = '\0';
    cmdTokenCount = 0;
    cmdErrorStatus = CMD_OK;
    overflowOccurred = false;
    for (uint8_t i = 0; i < CMD_MAX_TOKENS; i++)
    {
        cmdTokens[i] = NULL;
    }
}

/**
 * @brief  Convierte todos los caracteres de una cadena a mayúsculas para lograr
 *         comparaciones insensibles a mayúsculas/minúsculas (*case-insensitive*).
 * @param  str: Puntero a la cadena a convertir.
 * @retval None
 */
static void cmdToUpperCase(char * str)
{
    if (str == NULL)
    {
        return;
    }
    while (*str != '\0')
    {
        *str = (char)toupper((unsigned char)*str);
        str++;
    }
}

/**
 * @brief  Procesa la línea acumulada: filtra espacios iniciales, descarta comentarios
 *         iniciados con '#' o '//', y divide la cadena en tokens.
 * @param  None
 * @retval None
 */
static void cmdProcessLine(void)
{
    char * ptr = rxLineBuffer;

    /* Ignora espacios o tabulaciones al inicio de la línea */
    while (*ptr == ' ' || *ptr == '\t')
    {
        ptr++;
    }

    /* Línea vacía: se descarta silenciosamente sin emitir respuesta */
    if (*ptr == '\0')
    {
        cmdFsmReset();
        fsmState = CMD_IDLE;
        return;
    }

    /* Comentarios iniciados con '#' o '//': se descartan silenciosamente sin error */
    if (*ptr == '#' || (ptr[0] == '/' && ptr[1] == '/'))
    {
        cmdFsmReset();
        fsmState = CMD_IDLE;
        return;
    }

    /* Tokenización simple separada por espacios y tabuladores */
    cmdTokenCount = 0;
    char * token = strtok(ptr, " \t");
    while (token != NULL && cmdTokenCount < CMD_MAX_TOKENS)
    {
        cmdToUpperCase(token);
        cmdTokens[cmdTokenCount++] = token;
        token = strtok(NULL, " \t");
    }

    /* Si se recibieron más tokens de los permitidos, se considera argumento inválido */
    if (token != NULL)
    {
        cmdErrorStatus = CMD_ERR_ARG;
        fsmState = CMD_ERROR;
        return;
    }

    if (cmdTokenCount > 0)
    {
        fsmState = CMD_EXEC;
    }
    else
    {
        cmdFsmReset();
        fsmState = CMD_IDLE;
    }
}

/**
 * @brief  Evalúa los tokens procesados y ejecuta la acción asociada al comando.
 * @param  None
 * @retval None
 */
static void cmdExecute(void)
{
    if (cmdTokenCount == 0 || cmdTokens[0] == NULL)
    {
        cmdFsmReset();
        fsmState = CMD_IDLE;
        return;
    }

    /* =========================================================================
     * COMANDO: HELP
     * Muestra la lista de comandos disponibles y su sintaxis.
     * ========================================================================= */
    if (strcmp(cmdTokens[0], "HELP") == 0)
    {
        if (cmdTokenCount > 1)
        {
            cmdErrorStatus = CMD_ERR_ARG;
            fsmState = CMD_ERROR;
            return;
        }
        cmdPrintHelp();
        cmdFsmReset();
        fsmState = CMD_IDLE;
        return;
    }

    /* =========================================================================
     * COMANDO: LED [ON | OFF | TOGGLE]
     * Controla el estado del actuador LED utilizando las funciones desacopladas.
     * ========================================================================= */
    if (strcmp(cmdTokens[0], "LED") == 0)
    {
        if (cmdTokenCount != 2 || cmdTokens[1] == NULL)
        {
            cmdErrorStatus = CMD_ERR_ARG;
            fsmState = CMD_ERROR;
            return;
        }

        if (strcmp(cmdTokens[1], "ON") == 0)
        {
            ledOn();
            uartSendString((uint8_t *)CMD_MSG_OK);
            cmdFsmReset();
            fsmState = CMD_IDLE;
            return;
        }
        else if (strcmp(cmdTokens[1], "OFF") == 0)
        {
            ledOff();
            uartSendString((uint8_t *)CMD_MSG_OK);
            cmdFsmReset();
            fsmState = CMD_IDLE;
            return;
        }
        else if (strcmp(cmdTokens[1], "TOGGLE") == 0)
        {
            ledToggle();
            uartSendString((uint8_t *)CMD_MSG_OK);
            cmdFsmReset();
            fsmState = CMD_IDLE;
            return;
        }
        else
        {
            cmdErrorStatus = CMD_ERR_ARG;
            fsmState = CMD_ERROR;
            return;
        }
    }

    /* =========================================================================
     * COMANDO: STATUS
     * Lee y reporta el estado actual del actuador LED.
     * ========================================================================= */
    if (strcmp(cmdTokens[0], "STATUS") == 0)
    {
        if (cmdTokenCount != 1)
        {
            cmdErrorStatus = CMD_ERR_ARG;
            fsmState = CMD_ERROR;
            return;
        }

        if (ledGetStatus())
        {
            uartSendString((uint8_t *)CMD_MSG_LED_ON);
        }
        else
        {
            uartSendString((uint8_t *)CMD_MSG_LED_OFF);
        }

        cmdFsmReset();
        fsmState = CMD_IDLE;
        return;
    }

    /* =========================================================================
     * COMANDO AVANZADO: BAUD?
     * Retorna la velocidad actual de la UART seguida de \r\n.
     * ========================================================================= */
    if (strcmp(cmdTokens[0], "BAUD?") == 0)
    {
        if (cmdTokenCount != 1)
        {
            cmdErrorStatus = CMD_ERR_ARG;
            fsmState = CMD_ERROR;
            return;
        }

        char baudStr[CMD_RESP_BUFFER_SIZE];
        snprintf(baudStr, sizeof(baudStr), "%lu\r\n", (unsigned long)uartGetBaudRate());
        uartSendString((uint8_t *)baudStr);

        cmdFsmReset();
        fsmState = CMD_IDLE;
        return;
    }

    /* =========================================================================
     * COMANDO AVANZADO: BAUD=<valor> (acepta BAUD=115200, BAUD = 115200, etc.)
     * Reconfigura el baudrate en el rango 9600 a 921600 bps.
     * ========================================================================= */
    char * baudValStr = NULL;
    if (strncmp(cmdTokens[0], CMD_BAUD_STR_PREFIX, CMD_BAUD_PREFIX_LEN) == 0)
    {
        if (cmdTokenCount == 1)
        {
            baudValStr = cmdTokens[0] + CMD_BAUD_PREFIX_LEN;
        }
    }
    else if (strcmp(cmdTokens[0], "BAUD") == 0)
    {
        if (cmdTokenCount == 2 && cmdTokens[1][0] == '=')
        {
            baudValStr = cmdTokens[1] + 1;
        }
        else if (cmdTokenCount == 3 && strcmp(cmdTokens[1], "=") == 0)
        {
            baudValStr = cmdTokens[2];
        }
    }

    if (baudValStr != NULL)
    {
        char * endPtr = NULL;
        unsigned long requestedBaud = strtoul(baudValStr, &endPtr, CMD_BAUD_BASE);

        if (*baudValStr == '\0' || *endPtr != '\0' ||
            requestedBaud < UART_MIN_BAUDRATE || requestedBaud > UART_MAX_BAUDRATE)
        {
            cmdErrorStatus = CMD_ERR_ARG;
            fsmState = CMD_ERROR;
            return;
        }

        /* Responde OK antes de reconfigurar la velocidad para que llegue a la PC */
        uartSendString((uint8_t *)CMD_MSG_OK);

        if (!uartSetBaudRate((uint32_t)requestedBaud))
        {
            cmdErrorStatus = CMD_ERR_ARG;
            fsmState = CMD_ERROR;
            return;
        }

        cmdFsmReset();
        fsmState = CMD_IDLE;
        return;
    }

    /* Comando no reconocido */
    cmdErrorStatus = CMD_ERR_UNKNOWN;
    fsmState = CMD_ERROR;
}

/**
 * @brief  Emite por UART el mensaje de error correspondiente al código registrado.
 * @param  None
 * @retval None
 */
static void cmdHandleError(void)
{
    switch (cmdErrorStatus)
    {
        case CMD_ERR_OVERFLOW:
            uartSendString((uint8_t *)CMD_MSG_ERR_OVERFLOW);
            break;

        case CMD_ERR_UNKNOWN:
            uartSendString((uint8_t *)CMD_MSG_ERR_UNKNOWN);
            break;

        case CMD_ERR_ARG:
            uartSendString((uint8_t *)CMD_MSG_ERR_ARG);
            break;

        default:
            uartSendString((uint8_t *)CMD_MSG_ERR_SYNTAX);
            break;
    }

    cmdFsmReset();
    fsmState = CMD_IDLE;
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Inicializa la máquina de estados finitos (MEF) del parser de comandos,
 *         limpia los buffers internos y muestra el mensaje inicial de ayuda.
 * @param  None
 * @retval None
 */
void cmdParserInit(void)
{
    cmdFsmReset();
    fsmState = CMD_IDLE;
    cmdPrintHelp();
}

/**
 * @brief  Función de sondeo periódica (polling) no bloqueante. Debe ser llamada
 *         frecuentemente en el bucle principal. Lee los bytes entrantes por UART
 *         y actualiza los estados de la MEF.
 *
 * Detalle pedagógico de la MEF implementada:
 *
 * 1. ESTADO CMD_IDLE (Reposo):
 *    - Condición: El sistema espera el arribo de caracteres.
 *    - Evento: Se lee un byte con uartReceiveByte().
 *    - Guarda: Si el byte es '\r', '\n' o '\0', se descarta (previene saltos de línea huérfanos).
 *    - Acción: Almacena el primer carácter en rxLineBuffer[0], pone rxLineIndex = 1.
 *    - Transición: Pasa al estado CMD_RECEIVING.
 *
 * 2. ESTADO CMD_RECEIVING (Recepción de línea):
 *    - Condición: Se acumulan caracteres de la instrucción en curso.
 *    - Evento 2a (Fin de línea): Llega '\r', '\n' o '\0'.
 *      - Guarda: Si overflowOccurred == true -> Transición a CMD_ERROR (CMD_ERR_OVERFLOW).
 *      - Guarda: Si longitud válida -> Cierra la cadena con '\0' y transiciona a CMD_PROCESS.
 *    - Evento 2b (Carácter ordinario):
 *      - Guarda: rxLineIndex < CMD_MAX_LINE -> Almacena en rxLineBuffer[rxLineIndex++].
 *      - Guarda: rxLineIndex >= CMD_MAX_LINE -> Activa overflowOccurred = true (descarta excedente).
 *
 * 3. ESTADO CMD_PROCESS (Procesamiento y Tokenización):
 *    - Acción: Descarta espacios iniciales. Si es vacía o comentario ('#' o '//'), vuelve a CMD_IDLE.
 *    - Acción: Divide en tokens con strtok y normaliza a mayúsculas con toupper().
 *    - Guarda: Si hay exceso de tokens (> 3), pasa a CMD_ERROR (CMD_ERR_ARG).
 *    - Transición: Si hay comando, pasa a CMD_EXEC; si está vacía, a CMD_IDLE.
 *
 * 4. ESTADO CMD_EXEC (Ejecución):
 *    - Acción: Compara el comando (HELP, LED, STATUS, BAUD?, BAUD=) y ejecuta la acción.
 *    - Acción: Emite respuesta ("OK\r\n", "LED is ON\r\n", etc.) o programa código de error.
 *    - Transición: Retorna a CMD_IDLE si fue exitoso, o a CMD_ERROR ante fallas.
 *
 * 5. ESTADO CMD_ERROR (Manejo de Errores):
 *    - Acción: Emite el mensaje exacto según el código de error registrado ("ERROR: ...\r\n").
 *    - Acción: Limpia el búfer con cmdFsmReset().
 *    - Transición: Retorna a CMD_IDLE listo para el siguiente comando.
 *
 * @param  None
 * @retval None
 */
void cmdPoll(void)
{
    uint8_t rxByte = 0;
    uint8_t bytesProcessed = 0;

    /* Ingesta de hasta CMD_MAX_BYTES_PER_POLL bytes disponibles en el periférico serie sin bloquear la CPU */
    while (bytesProcessed < CMD_MAX_BYTES_PER_POLL && uartReceiveByte(&rxByte))
    {
        bytesProcessed++;
        switch (fsmState)
        {
            /* -----------------------------------------------------------------
             * ESTADO: CMD_IDLE
             * Espera el primer carácter válido para iniciar una nueva línea.
             * ----------------------------------------------------------------- */
            case CMD_IDLE:
                if (rxByte == '\r' || rxByte == '\n')
                {
                    /* Guarda: Ignora caracteres de fin de línea aislados en estado ocioso */
                    break;
                }
                /* Acción: Guarda el primer byte e inicializa el índice de recepción */
                rxLineBuffer[0] = (char)rxByte;
                rxLineIndex = 1;
                overflowOccurred = false;
                /* Transición: Comienza la recepción activa de la línea */
                fsmState = CMD_RECEIVING;
                break;

            /* -----------------------------------------------------------------
             * ESTADO: CMD_RECEIVING
             * Acumula caracteres en el buffer hasta encontrar el fin de línea.
             * ----------------------------------------------------------------- */
            case CMD_RECEIVING:
                if (rxByte == '\r' || rxByte == '\n')
                {
                    /* Evento: Se detectó el terminador de la trama (\r, \n o \r\n) */
                    if (overflowOccurred)
                    {
                        /* Guarda: La línea excedió CMD_MAX_LINE (64 caracteres) */
                        cmdErrorStatus = CMD_ERR_OVERFLOW;
                        fsmState = CMD_ERROR;
                    }
                    else
                    {
                        /* Guarda: Línea con longitud válida, se agrega el terminador nulo */
                        rxLineBuffer[rxLineIndex] = '\0';
                        fsmState = CMD_PROCESS;
                    }
                }
                else
                {
                    /* Evento: Carácter de datos */
                    if (rxLineIndex < (CMD_MAX_LINE - 1))
                    {
                        /* Guarda: Aún hay espacio dentro de los 64 bytes permitidos (incluye '\0') */
                        rxLineBuffer[rxLineIndex++] = (char)rxByte;
                    }
                    else
                    {
                        /* Guarda: Se superaron los 64 bytes; se activa la marca de desborde */
                        overflowOccurred = true;
                    }
                }
                break;

            default:
                break;
        }

        /* Si se completó una trama o se detectó error de desborde, se procede al procesamiento */
        if (fsmState == CMD_PROCESS || fsmState == CMD_ERROR)
        {
            break;
        }
    }

    /* -------------------------------------------------------------------------
     * TRANSICIONES DE EVALUACIÓN Y EJECUCIÓN
     * ------------------------------------------------------------------------- */

    /* ESTADO: CMD_PROCESS (Tokenización y filtrado de comentarios) */
    if (fsmState == CMD_PROCESS)
    {
        cmdProcessLine();
    }

    /* ESTADO: CMD_EXEC (Ejecución de la acción del comando parseado) */
    if (fsmState == CMD_EXEC)
    {
        cmdExecute();
    }

    /* ESTADO: CMD_ERROR (Reporte del mensaje de error exacto y reset) */
    if (fsmState == CMD_ERROR)
    {
        cmdHandleError();
    }
}

/**
 * @brief  Transmite por UART la lista completa de comandos soportados por el
 *         sistema junto con una breve descripción de cada uno.
 * @param  None
 * @retval None
 */
void cmdPrintHelp(void)
{
    uartSendString((uint8_t *)"\r\n--- Comandos disponibles ---\r\n");
    uartSendString((uint8_t *)"  HELP               : Muestra esta ayuda\r\n");
    uartSendString((uint8_t *)"  LED ON             : Enciende el LED LD2\r\n");
    uartSendString((uint8_t *)"  LED OFF            : Apaga el LED LD2\r\n");
    uartSendString((uint8_t *)"  LED TOGGLE         : Conmuta el LED LD2\r\n");
    uartSendString((uint8_t *)"  STATUS             : Informa el estado del LED LD2\r\n");
    uartSendString((uint8_t *)"  BAUD?              : Informa el baudrate actual\r\n");
    uartSendString((uint8_t *)"  BAUD=<9600-921600> : Modifica la velocidad serie\r\n");
    uartSendString((uint8_t *)"  # / //             : Lineas de comentario (ignoradas)\r\n");
    uartSendString((uint8_t *)"----------------------------\r\n");
}

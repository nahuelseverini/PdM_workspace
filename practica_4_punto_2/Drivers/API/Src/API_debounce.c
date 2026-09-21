/*
 * API_debounce.c
 */

#include "API_debounce.h"
#include "stm32f4xx_hal.h"

/* Private define ------------------------------------------------------------*/
/**
 * @brief Tiempo de retardo para el filtrado de rebotes (en milisegundos).
 */
#define DEBOUNCE_DELAY_MS 40

/**
 * @brief Definición del puerto y pin del pulsador de usuario.
 */
#define BUTTON_PORT GPIOC
#define BUTTON_PIN  GPIO_PIN_13

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief Estados posibles de la Máquina de Estados Finitos de antirrebote.
 */
typedef enum {
    BUTTON_UP,      /**< Pulsador liberado (estado estable) */
    BUTTON_FALLING, /**< Transición descendente, esperando tiempo de antirrebote */
    BUTTON_DOWN,    /**< Pulsador presionado (estado estable) */
    BUTTON_RAISING  /**< Transición ascendente, esperando tiempo de antirrebote */
} debounceState_t;

/* Private variables ---------------------------------------------------------*/
/**
 * @brief Estado actual de la MEF de antirrebote.
 */
static debounceState_t currentState;

/**
 * @brief Estructura de retardo no bloqueante para el tiempo de antirrebote.
 */
static delay_t debounceDelay;

/**
 * @brief Bandera que indica si la tecla fue presionada.
 */
static bool_t keyPressed;

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief  Acción ejecutada ante la confirmación de la pulsación de la tecla.
 * @param  None
 * @retval None
 */
static void buttonPressed(void);

/**
 * @brief  Acción ejecutada ante la confirmación de la liberación de la tecla.
 * @param  None
 * @retval None
 */
static void buttonReleased(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Acción ejecutada ante la confirmación de la pulsación de la tecla.
 * @param  None
 * @retval None
 */
static void buttonPressed(void)
{
	keyPressed = true;
}

/**
 * @brief  Acción ejecutada ante la confirmación de la liberación de la tecla.
 * @param  None
 * @retval None
 */
static void buttonReleased(void)
{
    /* Acción al soltar la tecla (no requiere acción adicional en esta práctica) */
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Carga el estado inicial de la MEF e inicializa el retardo de antirrebote.
 * @param  None
 * @retval None
 */
void debounceFSM_init(void)
{
	currentState = BUTTON_UP;
	keyPressed = false;
    delayInit(&debounceDelay, DEBOUNCE_DELAY_MS);
}

/**
 * @brief  Lee el pulsador, resuelve la transición de estados con retardo no bloqueante
 *         y dispara las funciones privadas de acción correspondientes.
 * @param  None
 * @retval None
 */
void debounceFSM_update(void)
{
    switch (currentState)
    {
        case BUTTON_UP:
            /* El pulsador de la placa es activo en bajo (RESET al presionar) */
            if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
            {
                /* Inicia el retardo no bloqueante de 40 ms */
                delayRead(&debounceDelay);
                currentState = BUTTON_FALLING;
            }
            break;

        case BUTTON_FALLING:
            /* Espera a que transcurra el tiempo de antirrebote */
            if (delayRead(&debounceDelay))
            {
                /* Segunda lectura: si sigue presionado, se confirma la pulsación */
                if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_RESET)
                {
                    buttonPressed();
                    currentState = BUTTON_DOWN;
                }
                else
                {
                    /* Falso positivo o rebote, regresa al estado inicial */
                	currentState = BUTTON_UP;
                }
            }
            break;

        case BUTTON_DOWN:
            /* Si se detecta nivel alto (SET), el usuario comenzó a soltar la tecla */
            if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_SET)
            {
                /* Inicia el retardo no bloqueante de 40 ms */
                delayRead(&debounceDelay);
                currentState = BUTTON_RAISING;
            }
            break;

        case BUTTON_RAISING:
            /* Espera a que transcurra el tiempo de antirrebote */
            if (delayRead(&debounceDelay))
            {
                /* Segunda lectura: si sigue en nivel alto, se confirma la liberación */
                if (HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN) == GPIO_PIN_SET)
                {
                    buttonReleased();
                    currentState = BUTTON_UP;
                }
                else
                {
                    /* Falso rebote al soltar, permanece presionado */
                	currentState = BUTTON_DOWN;
                }
            }
            break;

        default:
            /* Recuperación ante estado inválido */
            debounceFSM_init();
            break;
    }
}

/**
 * @brief  Consulta si la tecla fue presionada. Si se confirma la pulsación devuelve true
 *         y resetea el indicador interno a false.
 * @param  None
 * @retval bool_t: true si la tecla fue presionada, false en caso contrario.
 */
bool_t readKey(void)
{
    bool_t state = keyPressed;
    keyPressed = false;
    return state;
}

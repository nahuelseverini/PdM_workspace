/*
 * API_delay.c
 */

#include "API_delay.h"
#include "stm32f4xx_hal.h"
#include <stddef.h>

/* Private function prototypes -----------------------------------------------*/
static bool_t checkDelayPointer(delay_t * delay);
static bool_t checkDelayDuration(tick_t duration);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Verifica que el puntero a la estructura delay no sea nulo.
 * @param  delay: Puntero a evaluar.
 * @retval bool_t: true si es válido, false si es NULL.
 */
static bool_t checkDelayPointer(delay_t * delay)
{
    return (delay != NULL);
}

/**
 * @brief  Verifica que la duración especificada sea válida (mayor a cero).
 * @param  duration: Duración en milisegundos a evaluar.
 * @retval bool_t: true si es válida (> 0), false si es 0.
 */
static bool_t checkDelayDuration(tick_t duration)
{
    return (duration > 0);
}

/* Public functions ----------------------------------------------------------*/

void delayInit(delay_t * delay, tick_t duration)
{
    /* Validación de parámetros de entrada */
    if (checkDelayPointer(delay) && checkDelayDuration(duration))
    {
        /* Se asigna la duración pero no se inicia el conteo */
        delay->duration = duration;
        delay->running = false;
        delay->startTime = 0;
    }
}

bool_t delayRead(delay_t * delay)
{
    bool_t timeElapsed = false;

    /* Validación de parámetro de entrada */
    if (checkDelayPointer(delay))
    {
        if (!delay->running)
        {
            /* Si no estaba corriendo, se captura la marca de tiempo inicial y se arranca */
            delay->startTime = HAL_GetTick();
            delay->running = true;
        }
        else
        {
            /* Si ya está activo, se verifica si transcurrió el tiempo configurado */
            if ((HAL_GetTick() - delay->startTime) >= delay->duration)
            {
                /* El tiempo finalizó: se detiene el retardo y se indica que expiró */
                delay->running = false;
                timeElapsed = true;
            }
        }
    }
    return timeElapsed;
}

void delayWrite(delay_t * delay, tick_t duration)
{
    /* Validación de parámetros antes de actualizar la duración */
    if (checkDelayPointer(delay) && checkDelayDuration(duration))
    {
        delay->duration = duration;
    }
}

bool_t delayIsRunning(delay_t * delay)
{
    bool_t isRunning = false;

    /* Validación de parámetro antes de leer el estado */
    if (checkDelayPointer(delay))
    {
        /* Se devuelve una copia del campo running */
        isRunning = delay->running;
    }

    return isRunning;
}

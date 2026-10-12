/**
 * @file    bsp_delay.c
 * @brief   Tiempos: delay no bloqueante, tick en ms y espera en µs.
 * @author  Nahuel Severini
 */

#include "bsp_delay.h"
#include "stm32f4xx_hal.h"
#include <stddef.h>

void bsp_delay_init(bsp_delay_t *delay, bsp_tick_t duration_ms)
{
    if (delay == NULL) {
        return;
    }

    delay->duration = duration_ms;
    delay->running = false;
    delay->start_time = 0;
}

bool bsp_delay_read(bsp_delay_t *delay)
{
    if (delay == NULL) {
        return false;
    }

    /* Primera llamada: arranca el conteo */
    if (!delay->running) {
        delay->start_time = HAL_GetTick();
        delay->running = true;
        return false;
    }

    if ((HAL_GetTick() - delay->start_time) >= delay->duration) {
        delay->running = false;
        return true;
    }

    return false;
}

bsp_tick_t bsp_get_tick(void)
{
    return HAL_GetTick();
}

void bsp_delay_us_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void bsp_delay_us(uint32_t us)
{
    uint32_t cycles = us * (SystemCoreClock / 1000000U);
    uint32_t start = DWT->CYCCNT;

    while ((DWT->CYCCNT - start) < cycles) {
    }
}

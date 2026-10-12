/**
 * @file    bsp.c
 * @brief   Placa NUCLEO-F446RE: inicialización y tareas periódicas.
 * @author  Nahuel Severini
 */

#include "bsp.h"

void bsp_init(void)
{
    bsp_delay_us_init();
    bsp_led_init();
    bsp_btn_init();
}

void bsp_update(void)
{
    bsp_btn_update();
}

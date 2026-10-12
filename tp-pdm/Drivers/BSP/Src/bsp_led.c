/**
 * @file    bsp_led.c
 * @brief   LED verde LD2 de la placa (PA5).
 * @author  Nahuel Severini
 */

#include "bsp_led.h"
#include "main.h"

void bsp_led_init(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

void bsp_led_on(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

void bsp_led_off(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}

void bsp_led_toggle(void)
{
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

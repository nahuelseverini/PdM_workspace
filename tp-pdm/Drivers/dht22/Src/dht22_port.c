/**
 * @file    dht22_port.c
 * @brief   Parte del DHT22 que depende del hardware (pin PB5 y tiempos).
 * @author  Nahuel Severini
 */

#include "dht22_port.h"
#include "bsp_delay.h"
#include "stm32f4xx_hal.h"

#define DHT_PORT  GPIOB
#define DHT_PIN   GPIO_PIN_5

/* Ciclos del contador DWT que equivalen a 1 µs */
#define CYCLES_PER_US  (SystemCoreClock / 1000000U)

static bool pin_is_high(void)
{
    return HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_SET;
}

void dht22_port_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    dht22_port_set_input();
}

/* Open-drain: el micro solo baja la línea, la sube la resistencia pull-up */
void dht22_port_set_output(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = DHT_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DHT_PORT, &gpio);
}

void dht22_port_set_input(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = DHT_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT_PORT, &gpio);
}

void dht22_port_pin_write(bool level)
{
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void dht22_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void dht22_port_delay_us(uint32_t us)
{
    bsp_delay_us(us);
}

bool dht22_port_wait_pin_level(bool level, uint32_t timeout_us)
{
    uint32_t timeout_cycles = timeout_us * CYCLES_PER_US;
    uint32_t start = DWT->CYCCNT;

    while (pin_is_high() != level) {
        if ((DWT->CYCCNT - start) > timeout_cycles) {
            return false;
        }
    }
    return true;
}

uint32_t dht22_port_measure_high_pulse_us(uint32_t timeout_us)
{
    uint32_t timeout_cycles = timeout_us * CYCLES_PER_US;
    uint32_t start = DWT->CYCCNT;

    while (pin_is_high()) {
        if ((DWT->CYCCNT - start) > timeout_cycles) {
            return 0;
        }
    }
    return (DWT->CYCCNT - start) / CYCLES_PER_US;
}

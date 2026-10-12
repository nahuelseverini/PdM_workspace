/**
 * @file    lcd_port.c
 * @brief   Parte del LCD que depende del hardware (I2C1 en PB8/PB9 y tiempos).
 * @author  Nahuel Severini
 */

#include "lcd_port.h"
#include "bsp_delay.h"
#include "stm32f4xx_hal.h"
#include <stddef.h>

#define I2C_TIMEOUT_MS        100U
#define I2C_READY_TIMEOUT_MS  50U

static I2C_HandleTypeDef hi2c1;

bool lcd_port_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* PB8 = SCL y PB9 = SDA */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* I2C a 100 kHz */
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    return (HAL_I2C_Init(&hi2c1) == HAL_OK);
}

bool lcd_port_write(uint8_t dev_addr, uint8_t *p_data, uint16_t size)
{
    if (p_data == NULL || size == 0) {
        return false;
    }

    /* La HAL espera la dirección corrida un bit a la izquierda */
    return (HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(dev_addr << 1),
                                    p_data, size, I2C_TIMEOUT_MS) == HAL_OK);
}

bool lcd_port_is_ready(uint8_t dev_addr, uint32_t trials)
{
    return (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(dev_addr << 1),
                                  trials, I2C_READY_TIMEOUT_MS) == HAL_OK);
}

void lcd_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void lcd_port_delay_us(uint32_t us)
{
    bsp_delay_us(us);
}

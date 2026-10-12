/**
 * @file    bsp_led.h
 * @brief   LED verde LD2 de la placa (PA5).
 * @author  Nahuel Severini
 */

#ifndef BSP_LED_H_
#define BSP_LED_H_

/**
 * @brief  Deja el LED apagado.
 */
void bsp_led_init(void);

/**
 * @brief  Enciende el LED.
 */
void bsp_led_on(void);

/**
 * @brief  Apaga el LED.
 */
void bsp_led_off(void);

/**
 * @brief  Cambia el LED al estado contrario.
 */
void bsp_led_toggle(void);

#endif /* BSP_LED_H_ */

/**
 * @file    bsp.h
 * @brief   Placa NUCLEO-F446RE: inicialización y tareas periódicas.
 * @author  Nahuel Severini
 */

#ifndef BSP_H_
#define BSP_H_

#include "bsp_led.h"
#include "bsp_btn.h"
#include "bsp_delay.h"
#include "bsp_uart.h"

/**
 * @brief  Inicializa el LED, el botón y el contador de microsegundos.
 */
void bsp_init(void);

/**
 * @brief  Actualiza el antirrebote del botón. Se llama desde el loop principal.
 */
void bsp_update(void);

#endif /* BSP_H_ */

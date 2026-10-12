/**
 * @file    bsp_btn.h
 * @brief   Pulsador B1 de la placa (PC13) con antirrebote.
 * @author  Nahuel Severini
 */

#ifndef BSP_BTN_H_
#define BSP_BTN_H_

#include <stdbool.h>

/**
 * @brief  Deja el antirrebote en su estado inicial.
 */
void bsp_btn_init(void);

/**
 * @brief  Avanza la MEF del antirrebote. Se llama desde el loop principal.
 */
void bsp_btn_update(void);

/**
 * @brief  Indica si hubo una pulsación nueva. Al leerla se borra.
 * @retval true si se apretó el botón desde la última consulta.
 */
bool bsp_btn_get_pressed(void);

#endif /* BSP_BTN_H_ */

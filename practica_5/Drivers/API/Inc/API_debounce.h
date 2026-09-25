/*
 * API_debounce.h
 */

#ifndef API_DEBOUNCE_H_
#define API_DEBOUNCE_H_

#include "API_delay.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Inicializa la máquina de estados de antirrebote y el retardo no bloqueante.
 * @param  None
 * @retval None
 */
void debounceFSM_init(void);

/**
 * @brief  Actualiza periódicamente la MEF de antirrebote. Lee las entradas,
 *         resuelve las transiciones de estado y actualiza las variables internas.
 * @param  None
 * @retval None
 */
void debounceFSM_update(void);

/**
 * @brief  Consulta si la tecla fue presionada. Si se confirma la pulsación devuelve true
 *         y resetea el indicador interno a false.
 * @param  None
 * @retval bool_t: true si la tecla fue presionada, false en caso contrario.
 */
bool_t readKey(void);

#endif /* API_DEBOUNCE_H_ */

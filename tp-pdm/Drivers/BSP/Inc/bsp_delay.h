/**
 * @file    bsp_delay.h
 * @brief   Tiempos: delay no bloqueante, tick en ms y espera en µs.
 * @author  Nahuel Severini
 */

#ifndef BSP_DELAY_H_
#define BSP_DELAY_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t bsp_tick_t;

typedef struct {
    bsp_tick_t start_time;
    bsp_tick_t duration;
    bool       running;
} bsp_delay_t;

/**
 * @brief  Prepara un delay con la duración indicada. No lo arranca.
 * @param  delay       Delay a preparar.
 * @param  duration_ms Duración en milisegundos.
 */
void bsp_delay_init(bsp_delay_t *delay, bsp_tick_t duration_ms);

/**
 * @brief  Indica si ya pasó el tiempo del delay.
 * @note   La primera llamada arranca el conteo y devuelve false.
 *         Cuando devuelve true el delay se detiene; la siguiente llamada
 *         lo vuelve a arrancar.
 * @param  delay Delay a consultar.
 * @retval true si el tiempo ya pasó.
 */
bool bsp_delay_read(bsp_delay_t *delay);

/**
 * @brief  Devuelve los milisegundos transcurridos desde el arranque.
 */
bsp_tick_t bsp_get_tick(void);

/**
 * @brief  Activa el contador de ciclos que usa bsp_delay_us().
 */
void bsp_delay_us_init(void);

/**
 * @brief  Espera la cantidad de microsegundos indicada (bloquea).
 * @param  us Microsegundos a esperar.
 */
void bsp_delay_us(uint32_t us);

#endif /* BSP_DELAY_H_ */

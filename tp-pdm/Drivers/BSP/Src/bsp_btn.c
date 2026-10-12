/**
 * @file    bsp_btn.c
 * @brief   Pulsador B1 de la placa (PC13) con antirrebote.
 * @author  Nahuel Severini
 */

#include "bsp_btn.h"
#include "bsp_delay.h"
#include "main.h"

#define DEBOUNCE_MS  20U

typedef enum {
    BTN_UP,        /* suelto */
    BTN_FALLING,   /* se apretó, esperando que se estabilice */
    BTN_DOWN,      /* apretado */
    BTN_RISING     /* se soltó, esperando que se estabilice */
} btn_state_t;

static btn_state_t state = BTN_UP;
static bsp_delay_t debounce_delay;
static bool        pressed = false;

/* El botón B1 es activo en bajo: apretado = 0 */
static bool button_is_down(void)
{
    return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET;
}

/* La primera llamada a bsp_delay_read() arranca el conteo de los 20 ms */
static void start_debounce_timer(void)
{
    bsp_delay_read(&debounce_delay);
}

void bsp_btn_init(void)
{
    state = BTN_UP;
    pressed = false;
    bsp_delay_init(&debounce_delay, DEBOUNCE_MS);
}

void bsp_btn_update(void)
{
    switch (state)
    {
        case BTN_UP:
            if (button_is_down()) {
                start_debounce_timer();
                state = BTN_FALLING;
            }
            break;

        case BTN_FALLING:
            if (bsp_delay_read(&debounce_delay)) {
                if (button_is_down()) {
                    state = BTN_DOWN;
                    pressed = true;
                } else {
                    state = BTN_UP;
                }
            }
            break;

        case BTN_DOWN:
            if (!button_is_down()) {
                start_debounce_timer();
                state = BTN_RISING;
            }
            break;

        case BTN_RISING:
            if (bsp_delay_read(&debounce_delay)) {
                if (button_is_down()) {
                    state = BTN_DOWN;
                } else {
                    state = BTN_UP;
                }
            }
            break;

        default:
            state = BTN_UP;
            break;
    }
}

bool bsp_btn_get_pressed(void)
{
    bool result = pressed;
    pressed = false;
    return result;
}

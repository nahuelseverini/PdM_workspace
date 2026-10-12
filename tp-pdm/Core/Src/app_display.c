/**
 * @file    app_display.c
 * @brief   Pantalla LCD 16x2: muestra mediciones, umbral y estado.
 * @author  Nahuel Severini
 */

#include "app_display.h"
#include "app_climate.h"
#include "bsp_delay.h"
#include "lcd.h"
#include <stdio.h>

#define REFRESH_MS   250U    /* tiempo mínimo entre dos redibujados por cambio de valores */
#define SPLASH_MS    2000U   /* duración del mensaje de bienvenida */

static bool            lcd_ok = false;
static bool            showing_splash = false;
static uint32_t        splash_start_tick = 0;
static uint32_t        last_refresh_tick = 0;
static bool            must_redraw = true;

/* Lo último que se dibujó, para saber si algo cambió */
static climate_state_t shown_state = CLIMATE_STATE_INIT;
static float           shown_temp = 0.0f;
static float           shown_hum = 0.0f;
static float           shown_threshold = 0.0f;

/* Escribe un texto en una fila, rellenando con espacios hasta completar el ancho */
static void show_line(uint8_t row, const char *text)
{
    char line[LCD_COLUMNS + 1];
    snprintf(line, sizeof(line), "%-*.*s", LCD_COLUMNS, LCD_COLUMNS, text);

    lcd_set_cursor(row, 0);
    lcd_send_string(line);
}

static void show_measures(climate_state_t state, float temp, float hum)
{
    char text[LCD_COLUMNS + 8];

    snprintf(text, sizeof(text), "T:%.1f C H:%.1f%%", temp, hum);
    show_line(0, text);

    snprintf(text, sizeof(text), "ESTADO: %s", app_climate_state_name(state));
    show_line(1, text);
}

static void show_config(float threshold)
{
    char text[LCD_COLUMNS + 8];

    snprintf(text, sizeof(text), "%.1f C  (B1: +1)", threshold);

    show_line(0, "UMBRAL DE ALARMA");
    show_line(1, text);
}

static void show_screen(climate_state_t state, float temp, float hum, float threshold)
{
    switch (state)
    {
        case CLIMATE_STATE_NORMAL:
        case CLIMATE_STATE_ALARM:
            show_measures(state, temp, hum);
            break;

        case CLIMATE_STATE_CONFIG:
            show_config(threshold);
            break;

        case CLIMATE_STATE_ERROR:
        default:
            show_line(0, "T:--.- C H:--.-%");
            show_line(1, "ESTADO: ERROR");
            break;
    }
}

void app_display_init(void)
{
    lcd_ok = lcd_init();
    if (!lcd_ok) {
        return;
    }

    lcd_clear();
    show_line(0, "Monitor de clima");
    show_line(1, "Iniciando...");

    showing_splash = true;
    splash_start_tick = bsp_get_tick();
    must_redraw = true;
}

void app_display_update(void)
{
    if (!lcd_ok) {
        return;
    }

    uint32_t now = bsp_get_tick();

    /* Durante la bienvenida no se dibuja nada más */
    if (showing_splash) {
        if ((now - splash_start_tick) < SPLASH_MS) {
            return;
        }
        showing_splash = false;
        lcd_clear();
        must_redraw = true;
    }

    climate_state_t state = app_climate_get_state();
    float temp = app_climate_get_temperature();
    float hum = app_climate_get_humidity();
    float threshold = app_climate_get_threshold();

    /* El estado y el umbral se redibujan enseguida; las mediciones, cada 250 ms como mucho */
    bool urgent_change = (state != shown_state) || (threshold != shown_threshold);
    bool new_values = (temp != shown_temp) || (hum != shown_hum);
    bool time_passed = (now - last_refresh_tick) >= REFRESH_MS;

    if (must_redraw || urgent_change || (new_values && time_passed)) {
        show_screen(state, temp, hum, threshold);

        shown_state = state;
        shown_temp = temp;
        shown_hum = hum;
        shown_threshold = threshold;
        last_refresh_tick = now;
        must_redraw = false;
    }
}

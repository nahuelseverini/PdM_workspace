/**
 * @file    app_climate.c
 * @brief   Control de temperatura y humedad: MEF principal y alarma.
 * @author  Nahuel Severini
 */

#include "app_climate.h"
#include "dht22.h"
#include "bsp_led.h"
#include "bsp_btn.h"
#include "bsp_delay.h"

#define SAMPLE_INTERVAL_MS   2000U   /* cada cuánto se lee el sensor */
#define ALARM_HYSTERESIS_C   0.5f    /* la alarma se apaga 0.5 °C por debajo del umbral */
#define BLINK_NORMAL_MS      500U    /* LED lento (1 Hz) */
#define BLINK_ALARM_MS       100U    /* LED rápido (5 Hz) */
#define CONFIG_TIMEOUT_MS    5000U   /* sin tocar nada, CONFIG vuelve a NORMAL */

static climate_state_t current_state = CLIMATE_STATE_INIT;
static float           temp_threshold = APP_CLIMATE_DEFAULT_THRESHOLD;

static float           current_temp = 0.0f;
static float           current_hum = 0.0f;
static bool            sensor_ok = false;

static uint32_t        last_sample_tick = 0;
static uint32_t        last_blink_tick = 0;
static uint32_t        config_timeout_tick = 0;

static void enter_config_mode(void)
{
    current_state = CLIMATE_STATE_CONFIG;
    config_timeout_tick = bsp_get_tick();
}

static void blink_led(uint32_t now, uint32_t period_ms)
{
    if ((now - last_blink_tick) >= period_ms) {
        bsp_led_toggle();
        last_blink_tick = now;
    }
}

static void read_sensor_if_due(uint32_t now)
{
    if ((now - last_sample_tick) < SAMPLE_INTERVAL_MS) {
        return;
    }
    last_sample_tick = now;

    dht22_data_t data;
    sensor_ok = dht22_read(&data);

    if (sensor_ok) {
        current_temp = data.temperature;
        current_hum = data.humidity;
    }
}

void app_climate_init(void)
{
    dht22_init();

    current_state = CLIMATE_STATE_INIT;
    temp_threshold = APP_CLIMATE_DEFAULT_THRESHOLD;
    last_sample_tick = bsp_get_tick();
    last_blink_tick = bsp_get_tick();
}

void app_climate_update(void)
{
    uint32_t now = bsp_get_tick();
    bool button_pressed = bsp_btn_get_pressed();

    /* Muestreo periódico del sensor DHT22 cada 2000 ms */
    read_sensor_if_due(now);

    if (!sensor_ok && current_state != CLIMATE_STATE_INIT) {
        current_state = CLIMATE_STATE_ERROR;
    }

    /* MEF principal */
    switch (current_state)
    {
        case CLIMATE_STATE_INIT:
            bsp_led_off();
            if (sensor_ok) {
                current_state = CLIMATE_STATE_NORMAL;
            }
            break;

        case CLIMATE_STATE_NORMAL:
            blink_led(now, BLINK_NORMAL_MS);

            if (button_pressed) {
                enter_config_mode();
            } else if (current_temp >= temp_threshold) {
                current_state = CLIMATE_STATE_ALARM;
            }
            break;

        case CLIMATE_STATE_ALARM:
            blink_led(now, BLINK_ALARM_MS);

            if (button_pressed) {
                enter_config_mode();
            } else if (current_temp <= (temp_threshold - ALARM_HYSTERESIS_C)) {
                /* Se apaga un poco por debajo del umbral para que no titile */
                current_state = CLIMATE_STATE_NORMAL;
            }
            break;

        case CLIMATE_STATE_CONFIG:
            bsp_led_on();

            if (button_pressed) {
                app_climate_adjust_threshold(APP_CLIMATE_THRESHOLD_STEP);
            } else if ((now - config_timeout_tick) >= CONFIG_TIMEOUT_MS) {
                /* Sin tocar nada durante 5 s, vuelve al modo que corresponda */
                if (current_temp >= temp_threshold) {
                    current_state = CLIMATE_STATE_ALARM;
                } else {
                    current_state = CLIMATE_STATE_NORMAL;
                }
            }
            break;

        case CLIMATE_STATE_ERROR:
            bsp_led_off();

            if (button_pressed) {
                /* Fuerza una lectura nueva del sensor en el próximo ciclo */
                last_sample_tick = now - SAMPLE_INTERVAL_MS;
            }

            if (sensor_ok) {
                if (current_temp >= temp_threshold) {
                    current_state = CLIMATE_STATE_ALARM;
                } else {
                    current_state = CLIMATE_STATE_NORMAL;
                }
            }
            break;

        default:
            current_state = CLIMATE_STATE_INIT;
            break;
    }
}

climate_state_t app_climate_get_state(void)
{
    return current_state;
}

const char *app_climate_state_name(climate_state_t state)
{
    static const char *const names[] = { "INIT", "NORMAL", "CONFIG", "ALARM", "ERROR" };

    if ((unsigned int)state > CLIMATE_STATE_ERROR) {
        return "?";
    }
    return names[state];
}

float app_climate_get_threshold(void)
{
    return temp_threshold;
}

bool app_climate_set_threshold(float new_threshold)
{
    if (new_threshold < APP_CLIMATE_MIN_THRESHOLD ||
        new_threshold > APP_CLIMATE_MAX_THRESHOLD) {
        return false;
    }

    temp_threshold = new_threshold;
    enter_config_mode();
    return true;
}

void app_climate_adjust_threshold(float delta)
{
    float value = temp_threshold + delta;

    /* Si supera los límites, da la vuelta al otro extremo */
    if (value > APP_CLIMATE_MAX_THRESHOLD) {
        value = APP_CLIMATE_MIN_THRESHOLD;
    } else if (value < APP_CLIMATE_MIN_THRESHOLD) {
        value = APP_CLIMATE_MAX_THRESHOLD;
    }

    app_climate_set_threshold(value);
}

float app_climate_get_temperature(void)
{
    return current_temp;
}

float app_climate_get_humidity(void)
{
    return current_hum;
}

bool app_climate_is_sensor_ok(void)
{
    return sensor_ok;
}

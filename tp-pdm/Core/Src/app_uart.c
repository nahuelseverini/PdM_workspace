/**
 * @file    app_uart.c
 * @brief   Consola serie: comandos del usuario y envío periódico de mediciones.
 * @author  Nahuel Severini
 */

#include "app_uart.h"
#include "app_climate.h"
#include "bsp_uart.h"
#include "bsp_delay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TELEMETRY_INTERVAL_MS  2000U
#define RX_BUFFER_SIZE         64U
#define MAX_BYTES_PER_UPDATE   16U   /* para no quedarse leyendo y bloquear el loop */

#define KEY_BACKSPACE  0x08
#define KEY_DELETE     0x7F
#define KEY_ESCAPE     0x1B

typedef enum {
    UART_RX_IDLE,        /* línea vacía, esperando el primer carácter */
    UART_RX_RECEIVING,   /* armando la línea */
    UART_RX_ESCAPE,      /* ignorando una tecla especial (flechas, etc.) */
    UART_RX_PROCESS,     /* llegó Enter: hay que ejecutar la línea */
    UART_RX_ERROR        /* la línea es demasiado larga */
} rx_state_t;

static rx_state_t      rx_state = UART_RX_IDLE;
static char            rx_buffer[RX_BUFFER_SIZE];
static uint8_t         rx_index = 0;

static bool            telemetry_enabled = true;
static uint32_t        last_telemetry_tick = 0;
static climate_state_t last_state = CLIMATE_STATE_INIT;

/* Texto de un valor medido; si el sensor falla se muestra "--.-" */
static void format_value(char *buf, size_t size, float value, bool valid)
{
    if (valid) {
        snprintf(buf, size, "%.1f", value);
    } else {
        snprintf(buf, size, "--.-");
    }
}

static void print_threshold(const char *text)
{
    char msg[48];

    snprintf(msg, sizeof(msg), "%s %.1f C\r\n", text, app_climate_get_threshold());
    bsp_uart_send_string(msg);
}

/* Saca los espacios del principio y del final */
static char *trim(char *text)
{
    while (isspace((unsigned char)*text)) {
        text++;
    }

    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';

    return text;
}

/* Extrae el argumento que sigue al nombre del comando */
static char *command_argument(char *line, const char *word)
{
    size_t length = strlen(word);

    if (strncasecmp(line, word, length) != 0) {
        return NULL;
    }
    if (line[length] != '\0' && !isspace((unsigned char)line[length])) {
        return NULL;
    }

    char *arg = line + length;
    while (isspace((unsigned char)*arg)) {
        arg++;
    }
    return arg;
}

static void cmd_set(const char *arg)
{
    char *end;
    float value = strtof(arg, &end);

    if (end == arg || *end != '\0') {
        bsp_uart_send_string("Error: uso SET <temp>\r\n");
    } else if (app_climate_set_threshold(value)) {
        print_threshold("Umbral configurado:");
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Error: el umbral debe estar entre %d y %d C\r\n",
                 (int)APP_CLIMATE_MIN_THRESHOLD, (int)APP_CLIMATE_MAX_THRESHOLD);
        bsp_uart_send_string(msg);
    }
}

static void cmd_status(void)
{
    bool ok = app_climate_is_sensor_ok();
    char temp[16];
    char hum[16];
    char msg[256];

    format_value(temp, sizeof(temp), app_climate_get_temperature(), ok);
    format_value(hum, sizeof(hum), app_climate_get_humidity(), ok);

    snprintf(msg, sizeof(msg),
             "\r\n--- Estado del sistema ---\r\n"
             "  Estado      : %s\r\n"
             "  Temperatura : %s C\r\n"
             "  Humedad     : %s %%\r\n"
             "  Umbral      : %.1f C\r\n"
             "  Sensor      : %s\r\n"
             "  Telemetria  : %s\r\n"
             "--------------------------\r\n",
             app_climate_state_name(app_climate_get_state()),
             temp, hum, app_climate_get_threshold(),
             ok ? "OK" : "ERROR",
             telemetry_enabled ? "ON" : "OFF");
    bsp_uart_send_string(msg);
}

static void cmd_stream(const char *arg)
{
    if (strcasecmp(arg, "OFF") == 0) {
        telemetry_enabled = false;
        bsp_uart_send_string("Telemetria pausada.\r\n");
    } else if (strcasecmp(arg, "ON") == 0 || arg[0] == '\0') {
        telemetry_enabled = true;
        bsp_uart_send_string("Telemetria activada (cada 2 s).\r\n");
    } else {
        bsp_uart_send_string("Uso: STREAM ON|OFF\r\n");
    }
}

static void cmd_help(void)
{
    bsp_uart_send_string("\r\n--- Comandos ---\r\n"
                         "  +             : sube el umbral\r\n"
                         "  -             : baja el umbral\r\n"
                         "  SET <temp>    : fija el umbral\r\n"
                         "  STATUS        : muestra el estado del sistema\r\n"
                         "  STREAM ON|OFF : activa o pausa la telemetria\r\n"
                         "  HELP o ?      : muestra esta ayuda\r\n"
                         "----------------\r\n");
}

static void run_command(char *line)
{
    char *cmd = trim(line);

    if (cmd[0] == '\0') {
        return;
    }

    if (strcmp(cmd, "+") == 0 || strcmp(cmd, "-") == 0) {
        float step = APP_CLIMATE_THRESHOLD_STEP;
        if (cmd[0] == '-') {
            step = -step;
        }
        app_climate_adjust_threshold(step);
        print_threshold("Umbral:");
        return;
    }

    char *arg = command_argument(cmd, "SET");
    if (arg != NULL) {
        cmd_set(arg);
        return;
    }

    arg = command_argument(cmd, "STREAM");
    if (arg != NULL) {
        cmd_stream(arg);
        return;
    }

    if (strcasecmp(cmd, "STATUS") == 0) {
        cmd_status();
    } else if (strcasecmp(cmd, "HELP") == 0 || strcmp(cmd, "?") == 0) {
        cmd_help();
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "Comando desconocido '%s'. Escriba HELP.\r\n", cmd);
        bsp_uart_send_string(msg);
    }
}

static void send_telemetry(void)
{
    bool ok = app_climate_is_sensor_ok();
    char temp[16];
    char hum[16];
    char msg[96];

    format_value(temp, sizeof(temp), app_climate_get_temperature(), ok);
    format_value(hum, sizeof(hum), app_climate_get_humidity(), ok);

    snprintf(msg, sizeof(msg), "[TELEMETRIA] T: %s C | H: %s %% | Estado: %s\r\n",
             temp, hum, app_climate_state_name(app_climate_get_state()));
    bsp_uart_send_string(msg);
}

/* Avisa por la consola cada vez que cambia el estado de la MEF */
static void report_state_change(void)
{
    climate_state_t state = app_climate_get_state();

    if (state == last_state || state == CLIMATE_STATE_INIT) {
        return;
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "\r\n[ESTADO] %s -> %s\r\n",
             app_climate_state_name(last_state), app_climate_state_name(state));
    bsp_uart_send_string(msg);

    last_state = state;
}

static bool is_printable(uint8_t c)
{
    return (c >= ' ' && c <= '~');
}

/* Guarda el carácter en la línea y lo muestra en la consola */
static void add_char(uint8_t c)
{
    char echo[2] = { (char)c, '\0' };

    rx_buffer[rx_index++] = (char)c;
    bsp_uart_send_string(echo);
}

/* MEF que arma la línea que escribe el usuario, un byte por vez */
static void process_rx_byte(uint8_t byte)
{
    switch (rx_state)
    {
        case UART_RX_IDLE:
            if (byte == KEY_ESCAPE) {
                rx_state = UART_RX_ESCAPE;
            } else if (is_printable(byte)) {
                add_char(byte);
                rx_state = UART_RX_RECEIVING;
            }
            /* Con la línea vacía, Enter y borrar no hacen nada */
            break;

        case UART_RX_RECEIVING:
            if (byte == '\r' || byte == '\n') {
                bsp_uart_send_string("\r\n");
                rx_buffer[rx_index] = '\0';
                rx_state = UART_RX_PROCESS;
            } else if (byte == KEY_ESCAPE) {
                rx_state = UART_RX_ESCAPE;
            } else if (byte == KEY_BACKSPACE || byte == KEY_DELETE) {
                rx_index--;
                bsp_uart_send_string("\b \b");
                if (rx_index == 0) {
                    rx_state = UART_RX_IDLE;
                }
            } else if (is_printable(byte)) {
                if (rx_index < (RX_BUFFER_SIZE - 1)) {
                    add_char(byte);
                } else {
                    rx_state = UART_RX_ERROR;
                }
            }
            break;

        case UART_RX_ESCAPE:
            /* Teclas como las flechas mandan ESC [ A: se ignora hasta la letra final */
            if (byte != '[' && byte >= '@' && byte <= '~') {
                rx_state = (rx_index > 0) ? UART_RX_RECEIVING : UART_RX_IDLE;
            }
            break;

        case UART_RX_PROCESS:
        case UART_RX_ERROR:
        default:
            break;
    }

    /* PROCESS y ERROR se resuelven enseguida y se vuelve a IDLE */
    if (rx_state == UART_RX_PROCESS) {
        run_command(rx_buffer);
        rx_index = 0;
        rx_state = UART_RX_IDLE;
    } else if (rx_state == UART_RX_ERROR) {
        bsp_uart_send_string("\r\nError: la linea es demasiado larga.\r\n");
        rx_index = 0;
        rx_state = UART_RX_IDLE;
    }
}

void app_uart_init(void)
{
    rx_state = UART_RX_IDLE;
    rx_index = 0;
    telemetry_enabled = true;
    last_telemetry_tick = bsp_get_tick();
    last_state = CLIMATE_STATE_INIT;

    bsp_uart_send_string("\r\n--- Monitor de clima (NUCLEO-F446RE) ---\r\n"
                         "Escriba HELP para ver los comandos.\r\n\r\n");
}

void app_uart_update(void)
{
    uint32_t now = bsp_get_tick();

    report_state_change();

    uint8_t byte;
    uint8_t bytes_read = 0;
    while (bytes_read < MAX_BYTES_PER_UPDATE && bsp_uart_receive_byte(&byte)) {
        bytes_read++;
        process_rx_byte(byte);
    }

    if ((now - last_telemetry_tick) >= TELEMETRY_INTERVAL_MS) {
        last_telemetry_tick = now;

        climate_state_t state = app_climate_get_state();
        bool user_is_typing = (rx_index > 0);

        /* No se envía mientras el usuario escribe o configura el umbral */
        if (telemetry_enabled && state != CLIMATE_STATE_INIT &&
            state != CLIMATE_STATE_CONFIG && !user_is_typing) {
            send_telemetry();
        }
    }
}

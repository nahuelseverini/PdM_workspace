/**
 * @file    lcd.c
 * @brief   Display LCD 16x2 (HD44780) conectado por I2C.
 * @author  Nahuel Severini
 */

#include "lcd.h"
#include "lcd_port.h"
#include <stddef.h>

/* Bits del expansor PCF8574 que van al LCD */
#define PIN_RS         (1 << 0)   /* 0 = comando, 1 = dato */
#define PIN_EN         (1 << 2)   /* pulso que le avisa al LCD que lea */
#define PIN_BACKLIGHT  (1 << 3)   /* luz de fondo encendida */

/* Direcciones I2C posibles del módulo, según su chip */
#define LCD_ADDR_A     0x27
#define LCD_ADDR_B     0x3F
#define LCD_I2C_TRIES  3

/* Comandos del HD44780 */
#define LCD_CMD_CLEAR         0x01
#define LCD_CMD_ENTRY_MODE    0x06   /* el cursor avanza hacia la derecha */
#define LCD_CMD_DISPLAY_ON    0x0C   /* pantalla encendida, sin cursor */
#define LCD_CMD_FUNCTION_4BIT 0x28   /* 4 bits, 2 filas */
#define LCD_CMD_SET_CURSOR    0x80
#define LCD_INIT_8BIT         0x30   /* pasos del arranque */
#define LCD_INIT_4BIT         0x20

#define LCD_SECOND_ROW_ADDR   0x40

static uint8_t lcd_addr = LCD_ADDR_A;

/* El LCD trabaja con 4 bits por vez: se manda la parte alta del byte */
static void write_nibble(uint8_t nibble, bool is_data)
{
    uint8_t base = (nibble & 0xF0) | PIN_BACKLIGHT | (is_data ? PIN_RS : 0);

    /* Pulso en EN: se sube y se baja */
    uint8_t bytes[2] = { base | PIN_EN, base };

    lcd_port_write(lcd_addr, bytes, 2);
    lcd_port_delay_us(50);
}

static void send_byte(uint8_t byte, bool is_data)
{
    write_nibble(byte & 0xF0, is_data);          /* parte alta */
    write_nibble((byte << 4) & 0xF0, is_data);   /* parte baja */
}

static void send_command(uint8_t cmd)
{
    send_byte(cmd, false);

    /* Borrar la pantalla es lento */
    if (cmd == LCD_CMD_CLEAR) {
        lcd_port_delay_ms(2);
    }
}

bool lcd_init(void)
{
    if (!lcd_port_init()) {
        return false;
    }

    lcd_port_delay_ms(50);   /* el LCD necesita tiempo para arrancar */

    if (lcd_port_is_ready(LCD_ADDR_A, LCD_I2C_TRIES)) {
        lcd_addr = LCD_ADDR_A;
    } else if (lcd_port_is_ready(LCD_ADDR_B, LCD_I2C_TRIES)) {
        lcd_addr = LCD_ADDR_B;
    } else {
        return false;
    }

    /* Secuencia de arranque para pasar el LCD a modo de 4 bits */
    write_nibble(LCD_INIT_8BIT, false);
    lcd_port_delay_ms(5);
    write_nibble(LCD_INIT_8BIT, false);
    lcd_port_delay_ms(1);
    write_nibble(LCD_INIT_8BIT, false);
    lcd_port_delay_ms(1);
    write_nibble(LCD_INIT_4BIT, false);
    lcd_port_delay_ms(1);

    send_command(LCD_CMD_FUNCTION_4BIT);
    send_command(LCD_CMD_DISPLAY_ON);
    send_command(LCD_CMD_CLEAR);
    send_command(LCD_CMD_ENTRY_MODE);

    return true;
}

void lcd_clear(void)
{
    send_command(LCD_CMD_CLEAR);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    if (row >= LCD_ROWS) {
        row = LCD_ROWS - 1;
    }
    if (col >= LCD_COLUMNS) {
        col = LCD_COLUMNS - 1;
    }

    uint8_t address = col + ((row == 0) ? 0x00 : LCD_SECOND_ROW_ADDR);
    send_command(LCD_CMD_SET_CURSOR | address);
}

void lcd_send_string(const char *str)
{
    if (str == NULL) {
        return;
    }

    while (*str != '\0') {
        send_byte((uint8_t)*str, true);
        str++;
    }
}

/**
 * @file    app_display.h
 * @brief   Pantalla LCD 16x2: muestra mediciones, umbral y estado.
 * @author  Nahuel Severini
 */

#ifndef APP_DISPLAY_H_
#define APP_DISPLAY_H_

/**
 * @brief  Inicializa el LCD y muestra el mensaje de bienvenida.
 */
void app_display_init(void);

/**
 * @brief  Redibuja la pantalla cuando algo cambia. Se llama desde el loop principal.
 */
void app_display_update(void);

#endif /* APP_DISPLAY_H_ */

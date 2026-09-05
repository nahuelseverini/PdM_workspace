/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdint.h>     // Necesario para utilizar el tipo uint32_t.
#include <stdbool.h>    // Necesario para utilizar los valores booleanos true y false.

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/*
 * tick_t representa una marca de tiempo expresada en milisegundos.
 * Se utiliza uint32_t porque HAL_GetTick() devuelve un valor de este tipo.
 */
typedef uint32_t tick_t;

/*
 * bool_t se define para representar estados lógicos dentro del módulo
 * de retardos, por ejemplo si un retardo está actualmente en ejecución.
 */
typedef bool bool_t;

/*
 * La estructura delay_t almacena toda la información necesaria para
 * administrar un retardo sin detener la ejecución del programa.
 *
 * startTime guarda cuándo comenzó el retardo.
 * duration determina cuánto tiempo debe transcurrir.
 * running permite distinguir entre un retardo que todavía no comenzó
 * y uno que se encuentra actualmente contando tiempo.
 */
typedef struct
{
    tick_t startTime;
    tick_t duration;
    bool_t running;
} delay_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/*
 * Inicializa un retardo pero no inicia su temporización.
 * El conteo comienza recién cuando delayRead() es llamado.
 */
void delayInit(delay_t *delay, tick_t duration);

/*
 * Consulta de manera no bloqueante si el tiempo configurado ya transcurrió.
 * Permite que el procesador continúe realizando otras tareas mientras
 * el retardo está activo.
 */
bool_t delayRead(delay_t *delay);

/*
 * Permite modificar la duración de un retardo ya existente sin necesidad
 * de crear nuevamente la estructura.
 */
void delayWrite(delay_t *delay, tick_t duration);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

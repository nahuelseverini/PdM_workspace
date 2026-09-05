/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CANTIDAD_REPETICIONES 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

delay_t delay;

/*
 * Períodos de parpadeo utilizados en el patrón.
 *
 * 1000 ms -> 1 segundo
 * 200 ms  -> 200 milisegundos
 * 100 ms  -> 100 milisegundos
 */
tick_t periodos[] = {1000, 200, 100};

/*
 * Índice del período que estamos ejecutando.
 * 0 -> 1000 ms
 * 1 -> 200 ms
 * 2 -> 100 ms
 */
uint8_t indicePeriodo = 0;

/*
 * Cantidad de cambios de estado del LED realizados.
 */
uint8_t cantidadToggles = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /*
   * Inicializamos el delay con la mitad del primer período.
   *
   * El primer período es de 1000 ms y, como el ciclo de trabajo
   * es del 50%, el LED debe permanecer 500 ms en cada estado:
   *
   * 500 ms encendido + 500 ms apagado = 1000 ms de período.
   */
  delayInit(&delay, periodos[0] / 2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  /*
	   * delayRead() permite consultar periódicamente si transcurrió
	   * el tiempo configurado para el retardo, sin bloquear la ejecución
	   * del programa.
	   */
	  if (delayRead(&delay))
	  {
		  /*
		   * Cada vez que termina el retardo se cambia el estado del LED.
		   */
		  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

		  /*
		   * Contamos cada cambio de estado del LED.
		   */
		  cantidadToggles++;

		  /*
		   * Cada período completo requiere dos cambios de estado:
		   *
		   * 1 toggle -> encendido
		   * 1 toggle -> apagado
		   *
		   * Por lo tanto, para completar 5 períodos necesitamos
		   * 5 * 2 = 10 toggles.
		   */
		  if (cantidadToggles >= CANTIDAD_REPETICIONES * 2)
		  {
			  cantidadToggles = 0;

		      /*
		       * Pasamos al siguiente período del patrón.
		       */
		      indicePeriodo++;

		      /*
		       * Si terminamos el arreglo, volvemos al primer período.
		       */
		      if (indicePeriodo >= (sizeof(periodos) / sizeof(periodos[0])))
		      {
		    	  indicePeriodo = 0;
		      }

		      /*
		       * Configuramos la mitad del nuevo período.
		       *
		       * Esto permite mantener un ciclo de trabajo del 50%.
		       */
		      delayWrite(&delay, periodos[indicePeriodo] / 2);
		    }
	  }


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void delayInit(delay_t *delay, tick_t duration)
{
    /*
     * La inicialización solamente configura cuánto debe durar
     * el retardo. No se inicia todavía la medición del tiempo,
     * tal como requiere la consigna.
     */
    delay->duration = duration;
    delay->running = false;
}

bool_t delayRead(delay_t *delay)
{
    /*
     * Si el retardo no estaba ejecutándose, esta llamada establece
     * el instante de inicio.
     *
     * No se devuelve true porque todavía no transcurrió la duración
     * configurada.
     */
    if (!delay->running)
    {
        delay->startTime = HAL_GetTick();
        delay->running = true;

        return false;
    }

    /*
     * Se calcula cuánto tiempo transcurrió desde startTime.
     *
     * Esto permite esperar de forma no bloqueante: en lugar de detener
     * el programa durante la espera, se consulta periódicamente el
     * tiempo transcurrido.
     */
    if ((HAL_GetTick() - delay->startTime) >= delay->duration)
    {
        /*
         * El retardo terminó, por lo que se marca como no ejecutándose.
         * La próxima llamada a delayRead() comenzará una nueva cuenta.
         */
        delay->running = false;

        return true;
    }

    /*
     * Todavía no transcurrió el tiempo configurado.
     */
    return false;
}


void delayWrite(delay_t *delay, tick_t duration)
{
    /*
     * Se actualiza la duración para poder reutilizar el mismo retardo
     * con otro intervalo de tiempo.
     */
    delay->duration = duration;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

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
#define CZAS_JAZDY       2000
#define CZAS_OBROTU_90   400
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void zatrzymajSilniki(void);
void jedzNaprzod(void);
void jedzDoTylu(void);
void skrecWLewo(void);
void skrecWPrawo(void);
void sekwencja(void);

/*=============================================================================
  Zatrzymanie silników (0% duty)
=============================================================================*/
void zatrzymajSilniki(void)
{
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
}
/*=============================================================================
  Jazda do przodu: oba silniki do przodu + 80% PWM
=============================================================================*/
void jedzNaprzod(void)
{
  uint32_t predkosc = (htim3.Init.Period+1)*0.3;
  // ustawienie kierunku
  HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 1);
  HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
  HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 1);
  HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
  // ustawienie prędkości
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, predkosc);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, predkosc);
  // jazda
  HAL_Delay(CZAS_JAZDY);
  zatrzymajSilniki();
}

/*=============================================================================
  Jazda do tyłu: oba silniki do tyłu + 80% PWM
=============================================================================*/
void jedzDoTylu(void)
{
  uint32_t predkosc = (htim3.Init.Period + 1)*0.3;
  HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
  HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 1);
  HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
  HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 1);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, predkosc);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, predkosc);
  HAL_Delay(CZAS_JAZDY);
  zatrzymajSilniki();
}

/*=============================================================================
  Obrót w lewo o 90°: lewy silnik do tyłu, prawy do przodu
=============================================================================*/
void skrecWLewo(void)
{
  uint32_t predkosc = (htim3.Init.Period + 1)*0.3;
  HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 1);
  HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
  HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
  HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 1);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, predkosc);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, predkosc);
  HAL_Delay(CZAS_OBROTU_90);
  zatrzymajSilniki();
}

/*=============================================================================
  Obrót w prawo o 90°: lewy silnik do przodu, prawy do tyłu
=============================================================================*/
void skrecWPrawo(void)
{
  uint32_t predkosc = (htim3.Init.Period + 1)*0.3;
  HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
  HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 1);
  HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 1);
  HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, predkosc);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, predkosc);
  HAL_Delay(CZAS_OBROTU_90);
  zatrzymajSilniki();
}

void zatrzymajRobota(void)
{
  // Najpierw zatrzymaj PWM na obu kanałach
	HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
	HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
	HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
}

/*=============================================================================
  Wykonanie zadanej sekwencji ruchów:
  przód → prawo → przód → lewo → przód → tył → prawo → przód → lewo → tył
=============================================================================*/

void sekwencja(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  jedzNaprzod();
  skrecWLewo();
  jedzNaprzod();
  skrecWPrawo();
  jedzDoTylu();
  jedzNaprzod();
  skrecWPrawo();
  skrecWLewo();
  jedzDoTylu();
  skrecWPrawo();
  jedzNaprzod();
  zatrzymajRobota();
}

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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t guzikoSprawdz = 0;
  uint8_t guzikoSprawdzPrzed = 0;
  while (1){
	  guzikoSprawdz = HAL_GPIO_ReadPin(p_GPIO_Port, p_Pin);
	    if(guzikoSprawdz == 1 && guzikoSprawdzPrzed == 0){
	    	sekwencja();
	    }
	    guzikoSprawdzPrzed = guzikoSprawdz;
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 99;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, IN1_Pin|IN2_Pin|IN3_Pin|IN4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : p_Pin */
  GPIO_InitStruct.Pin = p_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(p_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IN1_Pin IN2_Pin IN3_Pin IN4_Pin */
  GPIO_InitStruct.Pin = IN1_Pin|IN2_Pin|IN3_Pin|IN4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

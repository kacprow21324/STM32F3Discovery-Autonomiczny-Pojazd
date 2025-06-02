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
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PREDKOSC_MIN     0
#define PREDKOSC_MAX   100
#define KROK_PREDKOSCI   10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint8_t Auto = 0;
volatile uint8_t procentPredkosci = 10;
char znak;
int ostatniKierunek = 0;
uint32_t lewaOdleglosc = 0;
uint32_t prawaOdleglosc = 0;
volatile uint8_t pokazOdleglosc = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void aktualizujPredkosc(void);
void zatrzymajSilniki(void);
void jedzNaprzod(void);
void jedzDoTylu(void);
void skrecWLewo(void);
void skrecWPrawo(void);
void zatrzymajRobota(void);
void jedzPoLinii(void);
void obslugaMenu(char znak);
uint32_t zmierzOdleglosc(GPIO_TypeDef* trigPort, uint16_t trigPin, GPIO_TypeDef* echoPort, uint16_t echoPin);
void wyswietlOdleglosc(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*=============================================================================
  Ustawianie PWM wg procentuPredkosci
=============================================================================*/
void aktualizujPredkosc(void)
{
   uint32_t period = htim3.Init.Period + 1;
   uint32_t predkosc = (period * procentPredkosci) / 100;
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, predkosc);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, predkosc);
}

/*=============================================================================
  Zatrzymanie silników
=============================================================================*/
void zatrzymajSilniki(void)
{
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
   __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
}

/*=============================================================================
  Jazda do przodu – oba silniki do przodu
=============================================================================*/
void jedzNaprzod(void)
{
   HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_RESET);
   aktualizujPredkosc();
}

/*=============================================================================
  Jazda do tyłu – oba silniki do tyłu
=============================================================================*/
void jedzDoTylu(void)
{
   HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_SET);
   aktualizujPredkosc();
}

/*=============================================================================
  Skręt w lewo: lewy silnik do tyłu, prawy do przodu
=============================================================================*/
void skrecWLewo(void)
{
   HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_SET);
   aktualizujPredkosc();
}

/*=============================================================================
  Skręt w prawo: lewy silnik do przodu, prawy do tyłu
=============================================================================*/
void skrecWPrawo(void)
{
   HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_RESET);
   aktualizujPredkosc();
}

/*=============================================================================
  Pełne zatrzymanie robota
=============================================================================*/
void zatrzymajRobota(void)
{
   zatrzymajSilniki();
   HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_RESET);
}

/*=============================================================================
  Pomiar odległości za pomocą czujnikow ultradźwiękowych
=============================================================================*/
uint32_t zmierzOdleglosc(GPIO_TypeDef* trigPort, uint16_t trigPin, GPIO_TypeDef* echoPort, uint16_t echoPin)
{
	uint32_t start = 0, stop = 0;
	uint32_t timeout = HAL_GetTick();

	HAL_GPIO_WritePin(trigPort, trigPin, 0);
	HAL_Delay(1);
	HAL_GPIO_WritePin(trigPort, trigPin, 1);
	__HAL_TIM_SET_COUNTER(&htim2, 0);

	while (__HAL_TIM_GET_COUNTER(&htim2) < 10);
	HAL_GPIO_WritePin(trigPort, trigPin, 0);

	while (HAL_GPIO_ReadPin(echoPort, echoPin) == 0)
	{
		if (HAL_GetTick() - timeout > 10) return 0;
	}
	start = __HAL_TIM_GET_COUNTER(&htim2);

	timeout = HAL_GetTick();
	while (HAL_GPIO_ReadPin(echoPort, echoPin) == 1)
	{
		if (HAL_GetTick() - timeout > 50) return 0;
	}
	stop = __HAL_TIM_GET_COUNTER(&htim2);

	uint32_t czas = (stop > start) ? (stop - start) : 0;

	return (uint32_t)(czas * 0.017f);
}

/*=============================================================================
  Jazda po linii z reakcją na czujniki IR oraz czujniki odległości
=============================================================================*/
void jedzPoLinii(void)
{
	lewaOdleglosc  = zmierzOdleglosc(GPIOB, TRIG1_Pin, GPIOA, GPIO_PIN_1);
	prawaOdleglosc = zmierzOdleglosc(GPIOB, TRIG2_Pin, GPIOA, GPIO_PIN_2);

	if (lewaOdleglosc < 15 || prawaOdleglosc < 15)
	{
		zatrzymajRobota();
		return;
	}

    aktualizujPredkosc();

    uint8_t skrajnieLewySensor  = HAL_GPIO_ReadPin(IR_LEFTMOST_D0_GPIO_Port, IR_LEFTMOST_D0_Pin);
    uint8_t lewySensor          = HAL_GPIO_ReadPin(IR_LEFT_D0_GPIO_Port,     IR_LEFT_D0_Pin);
    uint8_t centralnySensor     = HAL_GPIO_ReadPin(IR_CENTER_D0_GPIO_Port,   IR_CENTER_D0_Pin);
    uint8_t prawySensor         = HAL_GPIO_ReadPin(IR_RIGHT_D0_GPIO_Port,    IR_RIGHT_D0_Pin);
    uint8_t skrajniePrawySensor = HAL_GPIO_ReadPin(IR_RIGHTMOST_D0_GPIO_Port, IR_RIGHTMOST_D0_Pin);

    if (centralnySensor == 1)
    {
        jedzNaprzod();
        ostatniKierunek = 0;
    }
    else if (lewySensor == 1)
    {
        skrecWLewo();
        ostatniKierunek = 1;
    }
    else if (prawySensor == 1)
    {
        skrecWPrawo();
        ostatniKierunek = 2;
    }
    else if (skrajnieLewySensor == 1)
    {
        skrecWLewo();
        ostatniKierunek = 1;
    }
    else if (skrajniePrawySensor == 1)
    {
        skrecWPrawo();
        ostatniKierunek = 2;
    }
    else
    {
        jedzDoTylu();
        HAL_Delay(30);

        if (ostatniKierunek == 1)
        {
            skrecWLewo();
        }
        else if (ostatniKierunek == 2)
        {
            skrecWPrawo();
        }
        else
        {
            jedzNaprzod();
        }

        HAL_Delay(150);
        zatrzymajSilniki();
    }
}

/*=============================================================================
  Wyświetlanie odległości zmierzonych przez oba czujniki
=============================================================================*/
void wyswietlOdleglosc(void)
{
	lewaOdleglosc = zmierzOdleglosc(GPIOB, TRIG1_Pin, GPIOA, GPIO_PIN_1);
	prawaOdleglosc = zmierzOdleglosc(GPIOB, TRIG2_Pin, GPIOA, GPIO_PIN_2);
	char tekst[64];
	sprintf(tekst, "Lewy: %lu cm | Prawy: %lu cm\r\n", lewaOdleglosc, prawaOdleglosc);
	HAL_UART_Transmit(&huart1, (uint8_t*)tekst, strlen(tekst), HAL_MAX_DELAY);
}

/*=============================================================================
  MENU (UART)
=============================================================================*/
void obslugaMenu(char znak)
{
   switch (znak)
   {
   case 'M':
      if (procentPredkosci + KROK_PREDKOSCI <= PREDKOSC_MAX) {
         procentPredkosci += KROK_PREDKOSCI;
      } else {
         procentPredkosci = PREDKOSC_MAX;
      }
      aktualizujPredkosc();
      break;

   case 'N':
      if (procentPredkosci >= KROK_PREDKOSCI) {
         procentPredkosci -= KROK_PREDKOSCI;
      } else {
         procentPredkosci = PREDKOSC_MIN;
      }
      aktualizujPredkosc();
      break;

   case 'G':
      jedzNaprzod();
      break;

   case 'T':
      jedzDoTylu();
      break;

   case 'L':
      skrecWLewo();
      break;

   case 'P':
      skrecWPrawo();
      break;
   case 'S':
      zatrzymajRobota();
      break;
   case 'A':
      Auto = 1;
      break;
   case 'a':
      Auto = 0;
      zatrzymajRobota();
      break;
   case 'O':
	  pokazOdleglosc = 1;
      break;
   default:
      break;
   }
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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  aktualizujPredkosc();

  HAL_TIM_Base_Start(&htim2);
  HAL_UART_Receive_IT(&huart1, (uint8_t*)&znak, 1);

  char *w = "Robot Działa\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)w, strlen(w), HAL_MAX_DELAY);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
     if (Auto==1)
     {
    	 jedzPoLinii();
     }
     if (pokazOdleglosc==1)
     {
         wyswietlOdleglosc();
         pokazOdleglosc=0;
     }
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */
  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */
  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  /* USER CODE END TIM2_Init 2 */

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
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */
  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */
  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, IN1_Pin|IN2_Pin|IN3_Pin|IN4_Pin
                          |TRIG1_Pin|TRIG2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : IR_RIGHTMOST_D0_Pin IR_LEFTMOST_D0_Pin */
  GPIO_InitStruct.Pin = IR_RIGHTMOST_D0_Pin|IR_LEFTMOST_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : IN1_Pin IN2_Pin IN3_Pin IN4_Pin
                           TRIG1_Pin TRIG2_Pin */
  GPIO_InitStruct.Pin = IN1_Pin|IN2_Pin|IN3_Pin|IN4_Pin
                          |TRIG1_Pin|TRIG2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : IR_LEFT_D0_Pin IR_CENTER_D0_Pin IR_RIGHT_D0_Pin */
  GPIO_InitStruct.Pin = IR_LEFT_D0_Pin|IR_CENTER_D0_Pin|IR_RIGHT_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
   if (huart->Instance == USART1)
   {
      obslugaMenu(znak);
      HAL_UART_Receive_IT(&huart1, (uint8_t*)&znak, 1);
   }
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

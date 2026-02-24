/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common_def.h"
#include "temperature.h"
#include "oled_display.h"
#include "alarm_output.h"
#include "relay_control.h"
#include "safety_monitor.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  
  // Test UART output
  HAL_UART_Transmit(&huart1, (uint8_t*)"UART Test Start\r\n", 17, 100);
  HAL_Delay(100);
  
  printf("Printf Test OK\r\n");
  HAL_Delay(100);
  
  // Initialize Phase 4 - Temperature Detection and Fan Control
  printf("\r\n[System] Initializing temperature module...\r\n");
  Temperature_Init();
  printf("[System] Temperature module ready\r\n\r\n");
  HAL_Delay(100);
  
  // Phase 5 Test: OLED Display Module
  printf("\r\n========== Phase 5 Test ==========\r\n");
  printf("Testing OLED display module...\r\n\r\n");
  
  // Initialize OLED
  OLED_Init();
  printf("\r\n");
  HAL_Delay(500);
  
  // Run OLED test
  OLED_Test();
  
  printf("\r\n========== Phase 5 Test PASS ==========\r\n\r\n");
  HAL_Delay(10);
  
  // Phase 6 Test: Alarm Output Control
  printf("\r\n========== Phase 6 Test ==========\r\n");
  printf("Testing alarm output control module...\r\n\r\n");
  
  // Initialize alarm module
  Alarm_Init();
  HAL_Delay(500);
  
  // Test 1: Type-A error (1s pulse)
  printf("[Test 1] Type-A Error - 1s pulse test\r\n");
  Alarm_SetError(ERROR_TYPE_A);
  Alarm_PrintStatus();
  for(int i = 0; i < 5; i++) {
      printf("  [%d] Beep should pulse at 1s interval...\r\n", i+1);
      HAL_Delay(1000);
      Alarm_Update();
  }
  Alarm_ClearError(ERROR_TYPE_A);
  printf("  Type-A error cleared\r\n\r\n");
  HAL_Delay(1000);
  
  // Test 2: Type-B error (50ms pulse)
  printf("[Test 2] Type-B Error - 50ms pulse test\r\n");
  Alarm_SetError(ERROR_TYPE_B);
  Alarm_PrintStatus();
  for(int i = 0; i < 20; i++) {
      if (i % 5 == 0) printf("  [%d] Beep should pulse at 50ms interval...\r\n", i/5+1);
      HAL_Delay(100);
      Alarm_Update();
  }
  Alarm_ClearError(ERROR_TYPE_B);
  printf("  Type-B error cleared\r\n\r\n");
  HAL_Delay(1000);
  
  // Test 3: Type-K error (continuous beep)
  printf("[Test 3] Type-K Error - continuous beep test\r\n");
  Alarm_SetError(ERROR_TYPE_K);
  Alarm_PrintStatus();
  for(int i = 0; i < 5; i++) {
      printf("  [%d] Beep should be continuous...\r\n", i+1);
      HAL_Delay(1000);
      Alarm_Update();
  }
  Alarm_ClearError(ERROR_TYPE_K);
  printf("  Type-K error cleared\r\n\r\n");
  HAL_Delay(1000);
  
  // Test 4: Multiple error priority test
  printf("[Test 4] Multiple error priority test\r\n");
  printf("  Set Type-A error (1s pulse)...\r\n");
  Alarm_SetError(ERROR_TYPE_A);
  Alarm_Update();
  HAL_Delay(2000);
  
  printf("  Add Type-B error (50ms pulse) - should switch to 50ms...\r\n");
  Alarm_SetError(ERROR_TYPE_B);
  for(int i = 0; i < 10; i++) {
      HAL_Delay(100);
      Alarm_Update();
  }
  
  printf("  Add Type-K error (continuous) - should switch to continuous...\r\n");
  Alarm_SetError(ERROR_TYPE_K);
  for(int i = 0; i < 3; i++) {
      HAL_Delay(1000);
      Alarm_Update();
  }
  
  Alarm_PrintStatus();
  printf("  Clear all errors...\r\n");
  Alarm_ClearAll();
  Alarm_PrintStatus();
  HAL_Delay(1000);
  
  printf("\r\n========== Phase 6 Test PASS ==========\r\n\r\n");
  HAL_Delay(10);
  
  // Phase 7: Relay Control Module Test (Cycle Mode)
  printf("\r\n========== Phase 7 Test ==========\r\n");
  printf("Testing relay control module...\r\n\r\n");
  
  // Initialize relay module
  Relay_Init();
  HAL_Delay(500);
  
  // Initialize other modules
  Temperature_Init();
  Alarm_Init();
  
  printf("\r\n========== External Interrupt Test Mode ==========\r\n");
  printf("Testing K1_EN/K2_EN/K3_EN interrupt control...\r\n\r\n");
  printf("[Instructions]\r\n");
  printf("  K1_EN (PB9):  LOW=Open CH1,  HIGH=Close CH1\r\n");
  printf("  K2_EN (PB8):  LOW=Open CH2,  HIGH=Close CH2\r\n");
  printf("  K3_EN (PA15): LOW=Open CH3,  HIGH=Close CH3\r\n\r\n");
  printf("[Hardware Setup]\r\n");
  printf("  - Connect K1_EN/K2_EN/K3_EN to GND (LOW) or 3.3V (HIGH)\r\n");
  printf("  - Use jumper or switch to change levels\r\n");
  printf("  - Observe serial output and relay actions\r\n\r\n");
  printf("[Status Monitor Started - Change EN pins to test]\r\n\r\n");

  // Phase 8: Safety Monitor Module
  printf("\r\n========== Phase 8: Safety Monitor ==========\r\n");
  Safety_Init();
  printf("[Safety] Module ready, monitoring started\r\n\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t tick_20ms  = 0U;
  uint32_t tick_50ms  = 0U;
  uint32_t tick_100ms = 0U;
  uint32_t tick_1s    = 0U;

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    uint32_t now = HAL_GetTick();

    /* 20ms task: relay FSM update (500ms pulse timing) */
    if ((now - tick_20ms) >= 20U)
    {
      tick_20ms = now;
      Relay_Update();
    }

    /* 50ms task: alarm output drive (BEEP/ALARM pulse generation) */
    if ((now - tick_50ms) >= 50U)
    {
      tick_50ms = now;
      Alarm_Update();
    }

    /* 100ms task: safety monitor polling (error types A~O) */
    if ((now - tick_100ms) >= 100U)
    {
      tick_100ms = now;
      Safety_Update();
    }

    /* 1s task: temperature sampling, filtering, threshold check, fan RPM */
    if ((now - tick_1s) >= 1000U)
    {
      tick_1s = now;
      Temperature_Update();
      Temperature_1sHandler();
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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

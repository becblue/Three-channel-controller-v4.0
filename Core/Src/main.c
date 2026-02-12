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
  
  // 闂傚啳鍩栭锟�0婵炴潙顑堥惁顖炴晬濮橆偉顩柛娆欑到閻斺偓闁哄牜鍓熼埀顒佺煯娣囧﹤霉鐎ｎ厾妲�
  // Phase 1 Test: Common Definition Module
  DEBUG_PRINTF("\r\n========== Phase 1 Test ==========\r\n");
  DEBUG_PRINTF("Testing common_def module...\r\n\r\n");
  
  // Test 1: Module initialization
  CommonDef_Init();
  
  // Test 2: Error type enum test
  DEBUG_PRINTF("\r\n=== Error Type Test ===\r\n");
  DEBUG_PRINTF("ERROR_TYPE_NONE: %s\r\n", CommonDef_GetErrorString(ERROR_TYPE_NONE));
  DEBUG_PRINTF("ERROR_TYPE_A: %s\r\n", CommonDef_GetErrorString(ERROR_TYPE_A));
  DEBUG_PRINTF("ERROR_TYPE_K: %s\r\n", CommonDef_GetErrorString(ERROR_TYPE_K));
  DEBUG_PRINTF("ERROR_TYPE_O: %s\r\n", CommonDef_GetErrorString(ERROR_TYPE_O));
  
  // Test 3: Channel enum test
  DEBUG_PRINTF("\r\n=== Channel Test ===\r\n");
  DEBUG_PRINTF("CHANNEL_NONE: %s\r\n", CommonDef_GetChannelString(CHANNEL_NONE));
  DEBUG_PRINTF("CHANNEL_1: %s\r\n", CommonDef_GetChannelString(CHANNEL_1));
  DEBUG_PRINTF("CHANNEL_2: %s\r\n", CommonDef_GetChannelString(CHANNEL_2));
  DEBUG_PRINTF("CHANNEL_3: %s\r\n", CommonDef_GetChannelString(CHANNEL_3));
  
  // Test 4: System state enum test
  DEBUG_PRINTF("\r\n=== System State Test ===\r\n");
  DEBUG_PRINTF("SYSTEM_STATE_INIT: %s\r\n", CommonDef_GetSystemStateString(SYSTEM_STATE_INIT));
  DEBUG_PRINTF("SYSTEM_STATE_RUNNING: %s\r\n", CommonDef_GetSystemStateString(SYSTEM_STATE_RUNNING));
  DEBUG_PRINTF("SYSTEM_STATE_ERROR: %s\r\n", CommonDef_GetSystemStateString(SYSTEM_STATE_ERROR));
  
  // Test 5: Macro test
  DEBUG_PRINTF("\r\n=== Macro Test ===\r\n");
  DEBUG_PRINTF("TEMP_TO_CELSIUS(TEMP_THRESHOLD_LOW): %.1f C\r\n", TEMP_TO_CELSIUS(TEMP_THRESHOLD_LOW));
  DEBUG_PRINTF("TEMP_TO_CELSIUS(TEMP_THRESHOLD_HIGH): %.1f C\r\n", TEMP_TO_CELSIUS(TEMP_THRESHOLD_HIGH));
  DEBUG_PRINTF("LIMIT(150, 0, 100): %d\r\n", LIMIT(150, 0, 100));
  DEBUG_PRINTF("LIMIT(50, 0, 100): %d\r\n", LIMIT(50, 0, 100));
  
  // Test 6: Print full configuration
  CommonDef_PrintConfig();
  
  DEBUG_PRINTF("========== Phase 1 Test PASS ==========\r\n\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // 闂傚啳鍩栭锟�0婵炴潙顑堥惁顖炴晬濮橆厾妲ㄧ紒澶嬪笒瑜板倿鏌呮担椋庮伇婵炲棌鈧磭濡囬悹楦挎珪缁夌兘骞侀敓锟�
    // Phase 1 Test: Heartbeat every 5 seconds
    DEBUG_PRINTF("[%lu ms] System Running - Phase 1 Test OK\r\n", HAL_GetTick());
    HAL_Delay(5000);
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

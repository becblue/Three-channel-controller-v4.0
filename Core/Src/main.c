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
  
  // Basic UART test first
  HAL_UART_Transmit(&huart1, (uint8_t*)"UART Test Start\r\n", 17, 100);
  HAL_Delay(100);
  
  printf("Printf Test OK\r\n");
  HAL_Delay(100);
  
  // Initialize temperature module (Phase 4 - keep running in background)
  printf("\r\n[System] Initializing temperature module...\r\n");
  Temperature_Init();
  printf("[System] Temperature module ready\r\n\r\n");
  HAL_Delay(100);
  
  // Phase 5 Test: OLED Display Driver
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
  printf("娴嬭瘯鎶ヨ杈撳嚭鎺у埗妯″潡...\r\n\r\n");
  
  // 鍒濆鍖栨姤璀︽ā鍧�
  Alarm_Init();
  HAL_Delay(500);
  
  // 娴嬭瘯1锛欰绫诲紓甯革紙1绉掕剦鍐诧級
  printf("[Test 1] A绫诲紓甯� - 1绉掕剦鍐叉祴璇昞r\n");
  Alarm_SetError(ERROR_TYPE_A);
  Alarm_PrintStatus();
  for(int i = 0; i < 5; i++) {
      printf("  [%d] 铚傞福鍣ㄥ簲璇�1绉掕剦鍐�...\r\n", i+1);
      HAL_Delay(1000);
      Alarm_Update();
  }
  Alarm_ClearError(ERROR_TYPE_A);
  printf("  A绫诲紓甯稿凡娓呴櫎\r\n\r\n");
  HAL_Delay(1000);
  
  // 娴嬭瘯2锛欱绫诲紓甯革紙50ms鑴夊啿锛�
  printf("[Test 2] B绫诲紓甯� - 50ms鑴夊啿娴嬭瘯\r\n");
  Alarm_SetError(ERROR_TYPE_B);
  Alarm_PrintStatus();
  for(int i = 0; i < 20; i++) {
      if (i % 5 == 0) printf("  [%d] 铚傞福鍣ㄥ簲璇�50ms鑴夊啿...\r\n", i/5+1);
      HAL_Delay(100);
      Alarm_Update();
  }
  Alarm_ClearError(ERROR_TYPE_B);
  printf("  B绫诲紓甯稿凡娓呴櫎\r\n\r\n");
  HAL_Delay(1000);
  
  // 娴嬭瘯3锛欿绫诲紓甯革紙鎸佺画鍝嶏級
  printf("[Test 3] K绫诲紓甯� - 鎸佺画鍝嶆祴璇昞r\n");
  Alarm_SetError(ERROR_TYPE_K);
  Alarm_PrintStatus();
  for(int i = 0; i < 5; i++) {
      printf("  [%d] 铚傞福鍣ㄥ簲璇ユ寔缁搷...\r\n", i+1);
      HAL_Delay(1000);
      Alarm_Update();
  }
  Alarm_ClearError(ERROR_TYPE_K);
  printf("  K绫诲紓甯稿凡娓呴櫎\r\n\r\n");
  HAL_Delay(1000);
  
  // 娴嬭瘯4锛氬寮傚父浼樺厛绾ф祴璇�
  printf("[Test 4] 澶氬紓甯镐紭鍏堢骇娴嬭瘯\r\n");
  printf("  璁剧疆A绫诲紓甯革紙1绉掕剦鍐诧級...\r\n");
  Alarm_SetError(ERROR_TYPE_A);
  Alarm_Update();
  HAL_Delay(2000);
  
  printf("  鍐嶈缃瓸绫诲紓甯革紙50ms鑴夊啿锛�- 搴斿垏鎹㈠埌50ms鑴夊啿...\r\n");
  Alarm_SetError(ERROR_TYPE_B);
  for(int i = 0; i < 10; i++) {
      HAL_Delay(100);
      Alarm_Update();
  }
  
  printf("  鍐嶈缃甂绫诲紓甯革紙鎸佺画鍝嶏級- 搴斿垏鎹㈠埌鎸佺画鍝�...\r\n");
  Alarm_SetError(ERROR_TYPE_K);
  for(int i = 0; i < 3; i++) {
      HAL_Delay(1000);
      Alarm_Update();
  }
  
  Alarm_PrintStatus();
  printf("  娓呴櫎鎵€鏈夊紓甯�...\r\n");
  Alarm_ClearAll();
  Alarm_PrintStatus();
  HAL_Delay(1000);
  
  printf("\r\n========== Phase 6 Test PASS ==========\r\n\r\n");
  HAL_Delay(10);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // Update temperature in background
    Temperature_Update();
    
    // Phase 5 Test: Continuous OLED update with temperature info
    HAL_Delay(500);  // Changed from 5000ms to 500ms
    
    // Get temperature values and fan RPM
    int16_t t1, t2, t3;
    Temperature_GetValues(&t1, &t2, &t3);
    uint16_t fan_rpm = Temperature_GetFanRPM();
    
    // Update OLED display
    OLED_Clear();
    OLED_ShowString(0, 0, "System Running", OLED_FONT_6X8);
    
    // Show temperature on OLED
    char temp_str[20];
    snprintf(temp_str, sizeof(temp_str), "T1:%d.%dC", t1/10, abs(t1%10));
    OLED_ShowString(0, 2, temp_str, OLED_FONT_6X8);
    
    snprintf(temp_str, sizeof(temp_str), "T2:%d.%dC", t2/10, abs(t2%10));
    OLED_ShowString(0, 3, temp_str, OLED_FONT_6X8);
    
    snprintf(temp_str, sizeof(temp_str), "T3:%d.%dC", t3/10, abs(t3%10));
    OLED_ShowString(0, 4, temp_str, OLED_FONT_6X8);
    
    // Show fan RPM instead of percentage
    snprintf(temp_str, sizeof(temp_str), "FAN:%d RPM", fan_rpm);
    OLED_ShowString(0, 6, temp_str, OLED_FONT_6X8);
    
    OLED_Refresh();
    
    // UART output
    UART_PrintTimestamp();
    printf("T1:%d.%dC T2:%d.%dC T3:%d.%dC FAN:%d RPM - Phase 5 OK\r\n",
           t1/10, abs(t1%10), t2/10, abs(t2%10), t3/10, abs(t3%10),
           fan_rpm);
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

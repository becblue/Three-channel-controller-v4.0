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
#include "self_test.h"
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
static void update_oled_main_screen(void);
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
  printf("[System] Boot v4.0 - Three-channel Controller\r\n");

  Temperature_Init();
  OLED_Init();
  Alarm_Init();
  Relay_Init();
  Safety_Init();

  printf("[System] All modules initialized, starting self-test\r\n");
  SelfTest_Start();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t tick_20ms     = 0U;
  uint32_t tick_50ms     = 0U;
  uint32_t tick_100ms    = 0U;
  uint32_t tick_500ms    = 0U;
  uint32_t tick_1s       = 0U;
  uint32_t tick_selftest = 0U;

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    uint32_t now = HAL_GetTick();

    /* 20ms task: relay FSM (always runs; self-test correction pulses depend on this) */
    if ((now - tick_20ms) >= 20U)
    {
      tick_20ms = now;
      Relay_Update();
    }

    /* 50ms task: alarm output drive (always runs) */
    if ((now - tick_50ms) >= 50U)
    {
      tick_50ms = now;
      Alarm_Update();
    }

    if (SelfTest_IsRunning())
    {
      /* During self-test: advance state machine every 20ms */
      if ((now - tick_selftest) >= 20U)
      {
        tick_selftest = now;
        SelfTest_Update();
      }
    }
    else
    {
      /* 100ms: safety monitor */
      if ((now - tick_100ms) >= 100U)
      {
        tick_100ms = now;
        Safety_Update();
      }

      /* 500ms: OLED main screen refresh */
      if ((now - tick_500ms) >= 500U)
      {
        tick_500ms = now;
        update_oled_main_screen();
      }

      /* 1s: temperature update (1sHandler is called internally by Temperature_Update) */
      if ((now - tick_1s) >= 1000U)
      {
        tick_1s = now;
        Temperature_Update();
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

/**
 * @brief  Update OLED runtime main screen (called every 500ms)
 *
 * Screen layout (128x64, 6x8 font, 8 rows):
 *   Row 0 : Alarm status  - "Alarm: Normal" or "ERR: A B K ..."
 *   Row 1 : separator line at y=15
 *   Row 2 : Channel state - "CH:1=ON  2=OFF 3=OFF"
 *   Row 3-4: reserved
 *   Row 5 : separator line at y=47
 *   Row 6 : Temperatures  - "25.6/27.3/24.8C"
 *   Row 7 : Fan status    - "Fan:50%  RPM:1200"
 */
static void update_oled_main_screen(void)
{
  char      buf[22];
  uint16_t  err_flags;
  Channel_e active_ch;
  int16_t   t1, t2, t3;
  uint8_t   fan_spd;
  uint16_t  fan_rpm;
  uint8_t   idx;
  uint8_t   i;

  err_flags = Alarm_GetErrorFlags();
  active_ch = Relay_GetActiveChannel();
  Temperature_GetValues(&t1, &t2, &t3);
  fan_spd   = Temperature_GetFanSpeed();
  fan_rpm   = Temperature_GetFanRPM();

  /* -- Alarm area (row 0~1) -- */
  OLED_ClearArea(OLED_AREA_ALARM);
  if (err_flags == (uint16_t)ERROR_TYPE_NONE)
  {
    OLED_ShowString(0, 0, "Alarm: Normal       ", OLED_FONT_6X8);
  }
  else
  {
    idx = 0U;
    buf[idx++] = 'E'; buf[idx++] = 'R'; buf[idx++] = 'R'; buf[idx++] = ':';
    for (i = 0U; i < 15U; i++)
    {
      if (((err_flags >> i) & 0x01U) != 0U)
      {
        if (idx < 19U) { buf[idx++] = (char)('A' + (char)i); buf[idx++] = ' '; }
      }
    }
    while (idx < 20U) { buf[idx++] = ' '; }
    buf[20] = '\0';
    OLED_ShowString(0, 0, buf, OLED_FONT_6X8);
  }
  OLED_DrawLine(0, 15, 127, 15);

  /* -- Channel area (row 2~5) -- */
  OLED_ClearArea(OLED_AREA_CHANNEL);
  snprintf(buf, sizeof(buf), "CH:1=%-3s 2=%-3s 3=%-3s",
           (active_ch == CHANNEL_1) ? "ON" : "OFF",
           (active_ch == CHANNEL_2) ? "ON" : "OFF",
           (active_ch == CHANNEL_3) ? "ON" : "OFF");
  OLED_ShowString(0, 2, buf, OLED_FONT_6X8);
  OLED_DrawLine(0, 47, 127, 47);

  /* -- Temperature area (row 6~7) -- */
  OLED_ClearArea(OLED_AREA_TEMP);
  snprintf(buf, sizeof(buf), "%d.%d/%d.%d/%d.%dC",
           t1 / 10, abs(t1 % 10),
           t2 / 10, abs(t2 % 10),
           t3 / 10, abs(t3 % 10));
  OLED_ShowString(0, 6, buf, OLED_FONT_6X8);
  snprintf(buf, sizeof(buf), "Fan:%d%%  RPM:%d", fan_spd, fan_rpm);
  OLED_ShowString(0, 7, buf, OLED_FONT_6X8);

  OLED_Refresh();
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

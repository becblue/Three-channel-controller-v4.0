/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * @brief 格式化输出函�?
 * @param format: 格式化字符串
 * @param ...: 可变参数
 */
void UART_Printf(const char *format, ...);

/**
 * @brief 打印时间�?
 * @note 基于HAL_GetTick()输出毫秒级时间戳
 */
void UART_PrintTimestamp(void);

/**
 * @brief 日志输出函数
 * @param level: 日志级别字符串（�?"INFO", "WARNING", "ERROR"�?
 * @param msg: 日志消息内容
 */
void UART_Log(const char *level, const char *msg);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */


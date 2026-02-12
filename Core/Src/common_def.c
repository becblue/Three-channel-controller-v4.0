/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : common_def.c
  * @brief          : 通用定义实现文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本文件实现通用定义相关的函数
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "common_def.h"
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/**
 * @brief 获取异常类型字符串
 * @param error_type 异常类型
 * @return 异常类型描述字符串
 */
const char* CommonDef_GetErrorString(ErrorType_e error_type)
{
    switch(error_type)
    {
        case ERROR_TYPE_NONE: return "NONE";
        case ERROR_TYPE_A:    return "A-Enable Conflict";
        case ERROR_TYPE_B:    return "B-K1_1_STA Error";
        case ERROR_TYPE_C:    return "C-K2_1_STA Error";
        case ERROR_TYPE_D:    return "D-K3_1_STA Error";
        case ERROR_TYPE_E:    return "E-K1_2_STA Error";
        case ERROR_TYPE_F:    return "F-K2_2_STA Error";
        case ERROR_TYPE_G:    return "G-K3_2_STA Error";
        case ERROR_TYPE_H:    return "H-SW1_STA Error";
        case ERROR_TYPE_I:    return "I-SW2_STA Error";
        case ERROR_TYPE_J:    return "J-SW3_STA Error";
        case ERROR_TYPE_K:    return "K-NTC_1 Temp Error";
        case ERROR_TYPE_L:    return "L-NTC_2 Temp Error";
        case ERROR_TYPE_M:    return "M-NTC_3 Temp Error";
        case ERROR_TYPE_N:    return "N-SelfTest Error";
        case ERROR_TYPE_O:    return "O-Power Error";
        default:              return "Unknown Error";
    }
}

/**
 * @brief 获取通道名称字符串
 * @param channel 通道编号
 * @return 通道名称字符串
 */
const char* CommonDef_GetChannelString(Channel_e channel)
{
    switch(channel)
    {
        case CHANNEL_NONE: return "CH_NONE";
        case CHANNEL_1:    return "CH_1";
        case CHANNEL_2:    return "CH_2";
        case CHANNEL_3:    return "CH_3";
        default:           return "CH_Unknown";
    }
}

/**
 * @brief 获取系统状态字符串
 * @param state 系统状态
 * @return 系统状态描述字符串
 */
const char* CommonDef_GetSystemStateString(SystemState_e state)
{
    switch(state)
    {
        case SYSTEM_STATE_INIT:      return "INIT";
        case SYSTEM_STATE_LOGO:      return "LOGO";
        case SYSTEM_STATE_SELF_TEST: return "SELF_TEST";
        case SYSTEM_STATE_RUNNING:   return "RUNNING";
        case SYSTEM_STATE_ERROR:     return "ERROR";
        default:                     return "UNKNOWN";
    }
}

/**
 * @brief 通用定义模块初始化
 */
void CommonDef_Init(void)
{
    LOG_INFO("CommonDef module initializing...");
    
    // 当前版本无需特殊初始化
    
    LOG_INFO("CommonDef module initialized successfully");
}

/**
 * @brief 打印系统配置信息
 */
void CommonDef_PrintConfig(void)
{
    DEBUG_PRINTF("\r\n");
    DEBUG_PRINTF("========================================\r\n");
    DEBUG_PRINTF("  Three-Channel Controller v4.0\r\n");
    DEBUG_PRINTF("========================================\r\n");
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("=== System Configuration ===\r\n");
    DEBUG_PRINTF("MCU: STM32F103RCT6\r\n");
    DEBUG_PRINTF("Core Freq: 72MHz\r\n");
    DEBUG_PRINTF("HSE Clock: 8MHz\r\n");
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("=== Temperature Control ===\r\n");
    DEBUG_PRINTF("Normal Temp Threshold: %.1f C\r\n", TEMP_TO_CELSIUS(TEMP_THRESHOLD_LOW));
    DEBUG_PRINTF("Overheat Threshold: %.1f C\r\n", TEMP_TO_CELSIUS(TEMP_THRESHOLD_HIGH));
    DEBUG_PRINTF("Temp Hysteresis: %.1f C\r\n", TEMP_TO_CELSIUS(TEMP_HYSTERESIS));
    DEBUG_PRINTF("Fan PWM (Normal): %d%%\r\n", FAN_PWM_NORMAL);
    DEBUG_PRINTF("Fan PWM (High): %d%%\r\n", FAN_PWM_HIGH);
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("=== Timing Parameters ===\r\n");
    DEBUG_PRINTF("LOGO Display: %dms\r\n", TIME_LOGO_DISPLAY);
    DEBUG_PRINTF("Self-Test: %dms\r\n", TIME_SELF_TEST);
    DEBUG_PRINTF("Relay Pulse: %dms\r\n", TIME_RELAY_PULSE);
    DEBUG_PRINTF("Debounce Interval: %dms\r\n", TIME_DEBOUNCE_INTERVAL);
    DEBUG_PRINTF("Debounce Count: %d times\r\n", TIME_DEBOUNCE_COUNT);
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("=== Error Types ===\r\n");
    DEBUG_PRINTF("A - Enable Conflict\r\n");
    DEBUG_PRINTF("B~G - Relay Feedback Error\r\n");
    DEBUG_PRINTF("H~J - Switch Feedback Error\r\n");
    DEBUG_PRINTF("K~M - Temperature Error\r\n");
    DEBUG_PRINTF("N - Self-Test Error\r\n");
    DEBUG_PRINTF("O - Power Monitor Error\r\n");
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("=== Performance ===\r\n");
    DEBUG_PRINTF("Response Time: <%dms\r\n", SYSTEM_RESPONSE_TIME);
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("========================================\r\n");
    DEBUG_PRINTF("Configuration loaded successfully!\r\n");
    DEBUG_PRINTF("========================================\r\n");
    DEBUG_PRINTF("\r\n");
}

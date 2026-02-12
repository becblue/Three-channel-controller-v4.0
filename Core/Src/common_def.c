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
        case ERROR_TYPE_NONE: return "无异常";
        case ERROR_TYPE_A:    return "A-使能冲突";
        case ERROR_TYPE_B:    return "B-K1_1_STA异常";
        case ERROR_TYPE_C:    return "C-K2_1_STA异常";
        case ERROR_TYPE_D:    return "D-K3_1_STA异常";
        case ERROR_TYPE_E:    return "E-K1_2_STA异常";
        case ERROR_TYPE_F:    return "F-K2_2_STA异常";
        case ERROR_TYPE_G:    return "G-K3_2_STA异常";
        case ERROR_TYPE_H:    return "H-SW1_STA异常";
        case ERROR_TYPE_I:    return "I-SW2_STA异常";
        case ERROR_TYPE_J:    return "J-SW3_STA异常";
        case ERROR_TYPE_K:    return "K-NTC_1温度异常";
        case ERROR_TYPE_L:    return "L-NTC_2温度异常";
        case ERROR_TYPE_M:    return "M-NTC_3温度异常";
        case ERROR_TYPE_N:    return "N-自检异常";
        case ERROR_TYPE_O:    return "O-电源监控异常";
        default:              return "未知异常";
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
        case CHANNEL_NONE: return "全部关断";
        case CHANNEL_1:    return "通道1";
        case CHANNEL_2:    return "通道2";
        case CHANNEL_3:    return "通道3";
        default:           return "未知通道";
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
        case SYSTEM_STATE_INIT:      return "初始化";
        case SYSTEM_STATE_LOGO:      return "LOGO显示";
        case SYSTEM_STATE_SELF_TEST: return "系统自检";
        case SYSTEM_STATE_RUNNING:   return "正常运行";
        case SYSTEM_STATE_ERROR:     return "异常状态";
        default:                     return "未知状态";
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
    DEBUG_PRINTF("    三通道切换箱控制系统 v4.0\r\n");
    DEBUG_PRINTF("========================================\r\n");
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("【系统配置信息】\r\n");
    DEBUG_PRINTF("MCU型号: STM32F103RCT6\r\n");
    DEBUG_PRINTF("系统主频: 72MHz\r\n");
    DEBUG_PRINTF("外部晶振: 8MHz HSE\r\n");
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("【温度控制参数】\r\n");
    DEBUG_PRINTF("正常温度阈值: %.1f°C\r\n", TEMP_TO_CELSIUS(TEMP_THRESHOLD_LOW));
    DEBUG_PRINTF("过温告警阈值: %.1f°C\r\n", TEMP_TO_CELSIUS(TEMP_THRESHOLD_HIGH));
    DEBUG_PRINTF("温度回差: %.1f°C\r\n", TEMP_TO_CELSIUS(TEMP_HYSTERESIS));
    DEBUG_PRINTF("正常风扇PWM: %d%%\r\n", FAN_PWM_NORMAL);
    DEBUG_PRINTF("高温风扇PWM: %d%%\r\n", FAN_PWM_HIGH);
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("【时间参数】\r\n");
    DEBUG_PRINTF("LOGO显示时间: %dms\r\n", TIME_LOGO_DISPLAY);
    DEBUG_PRINTF("系统自检时间: %dms\r\n", TIME_SELF_TEST);
    DEBUG_PRINTF("继电器脉冲时长: %dms\r\n", TIME_RELAY_PULSE);
    DEBUG_PRINTF("防抖检测间隔: %dms\r\n", TIME_DEBOUNCE_INTERVAL);
    DEBUG_PRINTF("防抖检测次数: %d次\r\n", TIME_DEBOUNCE_COUNT);
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("【异常类型定义】\r\n");
    DEBUG_PRINTF("A - 使能冲突异常\r\n");
    DEBUG_PRINTF("B~G - 继电器状态反馈异常\r\n");
    DEBUG_PRINTF("H~J - 接触器状态反馈异常\r\n");
    DEBUG_PRINTF("K~M - 温度异常\r\n");
    DEBUG_PRINTF("N - 自检异常\r\n");
    DEBUG_PRINTF("O - 电源监控异常\r\n");
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("【性能指标】\r\n");
    DEBUG_PRINTF("系统响应时间要求: <%dms\r\n", SYSTEM_RESPONSE_TIME);
    DEBUG_PRINTF("\r\n");
    
    DEBUG_PRINTF("========================================\r\n");
    DEBUG_PRINTF("Configuration loaded successfully!\r\n");
    DEBUG_PRINTF("========================================\r\n");
    DEBUG_PRINTF("\r\n");
}

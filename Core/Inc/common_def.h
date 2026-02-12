/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : common_def.h
  * @brief          : 通用定义头文件 - 全局类型、枚举、常量和宏定义
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本文件包含项目的所有通用定义，包括：
  * - 异常类型枚举（15种异常A~O）
  * - 通道枚举
  * - 继电器状态枚举
  * - 蜂鸣器模式枚举
  * - 全局常量定义
  * - 调试宏定义
  *
  * GPIO引脚宏定义已由CubeMX生成在main.h中，无需重复定义
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_DEF_H
#define __COMMON_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 异常类型枚举（15种异常A~O）
 * 
 * 异常类型说明：
 * A - K1_EN、K2_EN、K3_EN使能冲突（多路同时低电平）
 * B - K1_1_STA工作异常（继电器1第一路状态反馈异常）
 * C - K2_1_STA工作异常（继电器2第一路状态反馈异常）
 * D - K3_1_STA工作异常（继电器3第一路状态反馈异常）
 * E - K1_2_STA工作异常（继电器1第二路状态反馈异常）
 * F - K2_2_STA工作异常（继电器2第二路状态反馈异常）
 * G - K3_2_STA工作异常（继电器3第二路状态反馈异常）
 * H - SW1_STA工作异常（接触器1状态反馈异常）
 * I - SW2_STA工作异常（接触器2状态反馈异常）
 * J - SW3_STA工作异常（接触器3状态反馈异常）
 * K - NTC_1温度异常（传感器1温度≥60℃）
 * L - NTC_2温度异常（传感器2温度≥60℃）
 * M - NTC_3温度异常（传感器3温度≥60℃）
 * N - 自检异常（上电自检过程中发现的异常）
 * O - 系统的电源监控保护功能
 */
typedef enum
{
    ERROR_TYPE_NONE = 0x0000,  // 无异常
    ERROR_TYPE_A    = 0x0001,  // 使能冲突
    ERROR_TYPE_B    = 0x0002,  // K1_1_STA异常
    ERROR_TYPE_C    = 0x0004,  // K2_1_STA异常
    ERROR_TYPE_D    = 0x0008,  // K3_1_STA异常
    ERROR_TYPE_E    = 0x0010,  // K1_2_STA异常
    ERROR_TYPE_F    = 0x0020,  // K2_2_STA异常
    ERROR_TYPE_G    = 0x0040,  // K3_2_STA异常
    ERROR_TYPE_H    = 0x0080,  // SW1_STA异常
    ERROR_TYPE_I    = 0x0100,  // SW2_STA异常
    ERROR_TYPE_J    = 0x0200,  // SW3_STA异常
    ERROR_TYPE_K    = 0x0400,  // NTC_1温度异常
    ERROR_TYPE_L    = 0x0800,  // NTC_2温度异常
    ERROR_TYPE_M    = 0x1000,  // NTC_3温度异常
    ERROR_TYPE_N    = 0x2000,  // 自检异常
    ERROR_TYPE_O    = 0x4000   // 电源监控异常
} ErrorType_e;

/**
 * @brief 通道枚举
 */
typedef enum
{
    CHANNEL_NONE = 0,  // 无通道（全部关断）
    CHANNEL_1    = 1,  // 通道1
    CHANNEL_2    = 2,  // 通道2
    CHANNEL_3    = 3   // 通道3
} Channel_e;

/**
 * @brief 继电器状态枚举
 */
typedef enum
{
    RELAY_STATE_OFF = 0,  // 继电器关闭
    RELAY_STATE_ON  = 1   // 继电器打开
} RelayState_e;

/**
 * @brief 蜂鸣器模式枚举
 */
typedef enum
{
    BEEP_MODE_OFF         = 0,  // 关闭
    BEEP_MODE_CONTINUOUS  = 1,  // 持续低电平（K~M类异常）
    BEEP_MODE_PULSE_50MS  = 2,  // 50ms间隔脉冲（B~J类异常）
    BEEP_MODE_PULSE_1S    = 3   // 1秒间隔脉冲（A、N、O类异常）
} BeepMode_e;

/**
 * @brief 系统状态枚举
 */
typedef enum
{
    SYSTEM_STATE_INIT       = 0,  // 初始化状态
    SYSTEM_STATE_LOGO       = 1,  // LOGO显示状态
    SYSTEM_STATE_SELF_TEST  = 2,  // 自检状态
    SYSTEM_STATE_RUNNING    = 3,  // 正常运行状态
    SYSTEM_STATE_ERROR      = 4   // 异常状态
} SystemState_e;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief 温度控制常量（单位：0.1℃）
 */
#define TEMP_THRESHOLD_LOW      350   // 35.0℃ - 正常/高温切换阈值
#define TEMP_THRESHOLD_HIGH     600   // 60.0℃ - 高温/过温切换阈值
#define TEMP_HYSTERESIS         20    // 2.0℃ - 温度回差

/**
 * @brief 温度阈值（考虑回差后的实际切换点）
 */
#define TEMP_LOW_TO_NORMAL      (TEMP_THRESHOLD_LOW - TEMP_HYSTERESIS)   // 33.0℃
#define TEMP_HIGH_TO_OVERHEAT   TEMP_THRESHOLD_HIGH                      // 60.0℃
#define TEMP_OVERHEAT_TO_HIGH   (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS) // 58.0℃

/**
 * @brief 风扇PWM占空比常量（0-100）
 */
#define FAN_PWM_NORMAL          50    // 正常温度下的占空比：50%
#define FAN_PWM_HIGH            95    // 高温/过温下的占空比：95%

/**
 * @brief 时间常量（单位：ms）
 */
#define TIME_LOGO_DISPLAY       2000  // LOGO显示时间：2秒
#define TIME_SELF_TEST          3000  // 自检时间：3秒
#define TIME_RELAY_PULSE        500   // 继电器脉冲时长：500ms
#define TIME_DEBOUNCE_INTERVAL  50    // 防抖检测间隔：50ms
#define TIME_DEBOUNCE_COUNT     3     // 防抖检测次数：3次

/**
 * @brief 防抖检测常量
 */
#define DEBOUNCE_CHECK_TIMES    3     // 连续检测次数

/**
 * @brief ADC相关常量
 */
#define ADC_CHANNEL_COUNT       3     // ADC通道数量（3路NTC）
#define ADC_VREF                3300  // ADC参考电压：3.3V (单位：mV)
#define ADC_RESOLUTION          4096  // ADC分辨率：12位 (0-4095)

/**
 * @brief 系统性能指标
 */
#define SYSTEM_RESPONSE_TIME    500   // 系统响应时间要求：<500ms

/**
 * @brief 调试开关
 */
#define DEBUG_ENABLE            1     // 1-使能调试输出，0-禁用调试输出

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief 调试宏定义
 */
#if DEBUG_ENABLE
    #define DEBUG_PRINTF(fmt, ...)      printf(fmt, ##__VA_ARGS__)
    #define LOG_INFO(msg)               printf("[INFO] %s\r\n", msg)
    #define LOG_WARNING(msg)            printf("[WARNING] %s\r\n", msg)
    #define LOG_ERROR(msg)              printf("[ERROR] %s\r\n", msg)
#else
    #define DEBUG_PRINTF(fmt, ...)      ((void)0)
    #define LOG_INFO(msg)               ((void)0)
    #define LOG_WARNING(msg)            ((void)0)
    #define LOG_ERROR(msg)              ((void)0)
#endif

/**
 * @brief 位操作宏
 * 注意：SET_BIT、CLEAR_BIT、READ_BIT已在stm32f1xx.h中定义，无需重复定义
 * 这里只定义HAL库未提供的TOGGLE_BIT宏
 */
#define TOGGLE_BIT(reg, bit)    ((reg) ^= (bit))

/**
 * @brief 数值范围限制宏
 */
#define LIMIT(x, min, max)      (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))

/**
 * @brief 数组大小计算宏
 */
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief 温度转换宏（0.1℃ ↔ ℃）
 */
#define TEMP_TO_CELSIUS(t)      ((float)(t) / 10.0f)
#define CELSIUS_TO_TEMP(c)      ((int16_t)((c) * 10))

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 获取异常类型字符串
 * @param error_type 异常类型
 * @return 异常类型描述字符串
 */
const char* CommonDef_GetErrorString(ErrorType_e error_type);

/**
 * @brief 获取通道名称字符串
 * @param channel 通道编号
 * @return 通道名称字符串
 */
const char* CommonDef_GetChannelString(Channel_e channel);

/**
 * @brief 获取系统状态字符串
 * @param state 系统状态
 * @return 系统状态描述字符串
 */
const char* CommonDef_GetSystemStateString(SystemState_e state);

/**
 * @brief 通用定义模块初始化
 */
void CommonDef_Init(void);

/**
 * @brief 打印系统配置信息
 */
void CommonDef_PrintConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_DEF_H */

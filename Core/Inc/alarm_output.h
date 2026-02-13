/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : alarm_output.h
  * @brief          : 报警输出控制模块头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本模块实现多级报警控制功能：
  * 1. ALARM引脚控制 - 任何异常时输出低电平
  * 2. 蜂鸣器分级控制：
  *    - K~M类异常（温度）：持续低电平（最高优先级）
  *    - B~J类异常（状态反馈）：50ms间隔脉冲
  *    - A、N、O类异常（冲突/自检）：1秒间隔脉冲
  * 3. 异常标志位图管理（16位）
  * 4. 基于定时器的非阻塞脉冲生成
  *
  * 硬件连接：
  * - ALARM: PB4（输出，低电平有效）
  * - BEEP:  PB3（输出，低电平有效）
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ALARM_OUTPUT_H
#define __ALARM_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "common_def.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 报警管理器数据结构
 */
typedef struct {
    uint16_t error_flags;          // 异常标志位图（16位，每位对应一种异常）
    BeepMode_e beep_mode;          // 当前蜂鸣器模式
    uint32_t beep_timer;           // 蜂鸣器定时器（用于脉冲生成）
    bool beep_state;               // 蜂鸣器当前状态（true-响，false-静）
    bool alarm_active;             // ALARM引脚状态（true-低电平告警）
    bool initialized;              // 初始化标志
} Alarm_Manager_t;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief 蜂鸣器脉冲周期（单位：ms）
 */
#define BEEP_PULSE_PERIOD_50MS      100    // 50ms间隔脉冲周期（50ms响+50ms静）
#define BEEP_PULSE_PERIOD_1S        2000   // 1秒间隔脉冲周期（1s响+1s静）

/**
 * @brief 蜂鸣器脉冲占空比（单位：ms）
 */
#define BEEP_PULSE_ON_50MS          50     // 50ms脉冲响声时长
#define BEEP_PULSE_ON_1S            1000   // 1秒脉冲响声时长

/**
 * @brief 异常优先级分组（用于确定蜂鸣器模式）
 */
#define ERROR_GROUP_TEMP            (ERROR_TYPE_K | ERROR_TYPE_L | ERROR_TYPE_M)  // 温度异常（持续）
#define ERROR_GROUP_FEEDBACK        (ERROR_TYPE_B | ERROR_TYPE_C | ERROR_TYPE_D | \
                                     ERROR_TYPE_E | ERROR_TYPE_F | ERROR_TYPE_G | \
                                     ERROR_TYPE_H | ERROR_TYPE_I | ERROR_TYPE_J)  // 状态反馈异常（50ms脉冲）
#define ERROR_GROUP_CONFLICT        (ERROR_TYPE_A | ERROR_TYPE_N | ERROR_TYPE_O)  // 冲突/自检异常（1s脉冲）

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief 异常标志检查宏
 */
#define ALARM_HAS_ERROR(flags)          ((flags) != ERROR_TYPE_NONE)
#define ALARM_HAS_TEMP_ERROR(flags)     (((flags) & ERROR_GROUP_TEMP) != 0)
#define ALARM_HAS_FEEDBACK_ERROR(flags) (((flags) & ERROR_GROUP_FEEDBACK) != 0)
#define ALARM_HAS_CONFLICT_ERROR(flags) (((flags) & ERROR_GROUP_CONFLICT) != 0)

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  报警模块初始化
 * @note   初始化GPIO、清除所有异常标志
 * @retval None
 */
void Alarm_Init(void);

/**
 * @brief  设置异常标志
 * @param  error_type: 异常类型（ErrorType_e枚举值）
 * @note   可以同时设置多个异常（使用位或操作）
 * @retval None
 */
void Alarm_SetError(ErrorType_e error_type);

/**
 * @brief  清除异常标志
 * @param  error_type: 异常类型（ErrorType_e枚举值）
 * @note   可以同时清除多个异常（使用位或操作）
 * @retval None
 */
void Alarm_ClearError(ErrorType_e error_type);

/**
 * @brief  检查是否有任何异常
 * @retval true-有异常，false-无异常
 */
bool Alarm_HasError(void);

/**
 * @brief  获取当前异常标志位图
 * @retval 异常标志位图（16位）
 */
uint16_t Alarm_GetErrorFlags(void);

/**
 * @brief  获取当前蜂鸣器模式
 * @retval 蜂鸣器模式（BeepMode_e枚举值）
 */
BeepMode_e Alarm_GetBeepMode(void);

/**
 * @brief  更新报警状态（非阻塞）
 * @note   定时调用此函数更新ALARM引脚和蜂鸣器状态
 *         建议调用频率：10-50Hz（20-100ms）
 * @retval None
 */
void Alarm_Update(void);

/**
 * @brief  打印报警状态（调试用）
 * @note   通过串口输出当前异常标志、蜂鸣器模式等信息
 * @retval None
 */
void Alarm_PrintStatus(void);

/**
 * @brief  清除所有异常标志（强制复位）
 * @note   用于系统复位或紧急情况
 * @retval None
 */
void Alarm_ClearAll(void);

/**
 * @brief  获取报警管理器指针（调试用）
 * @retval 报警管理器数据结构指针
 */
const Alarm_Manager_t* Alarm_GetManager(void);

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_OUTPUT_H */

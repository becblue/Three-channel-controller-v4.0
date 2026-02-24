/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : safety_monitor.h
  * @brief          : 安全监控模块头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本模块实现15种异常的实时检测与报警联动：
  *
  * 设计原则：
  * 1. Alarm 是唯一错误状态源，本模块直接调用 Alarm_SetError/ClearError，
  *    不维护任何内部 active_errors 位图。
  * 2. 每次 Safety_Update() 调用时，每类异常均根据当前 GPIO 实时状态
  *    独立执行 Set 或 Clear，不存在单向逻辑。
  * 3. 期望值直接来自 K_EN 引脚电平，无真值表。
  * 4. 当任意通道处于 500ms 脉冲切换过渡期时，B~J 类检测全部跳过，
  *    防止继电器动作期间产生误报警。
  * 5. 模块内部仅保留 2 个字段，无冗余状态。
  *
  * 异常类型（来自 common_def.h ErrorType_e）：
  * A  - K_EN 使能冲突（≥2路同时为低）
  * B  - K1_1_STA 继电器反馈异常
  * C  - K2_1_STA 继电器反馈异常
  * D  - K3_1_STA 继电器反馈异常
  * E  - K1_2_STA 继电器反馈异常
  * F  - K2_2_STA 继电器反馈异常
  * G  - K3_2_STA 继电器反馈异常
  * H  - SW1_STA 接触器反馈异常
  * I  - SW2_STA 接触器反馈异常
  * J  - SW3_STA 接触器反馈异常
  * K  - NTC1 过温（≥60°C）
  * L  - NTC2 过温（≥60°C）
  * M  - NTC3 过温（≥60°C）
  * N  - 自检失败（由 self_test 模块主动调用接口控制）
  * O  - DC 电源掉电（DC_CTRL 为低）
  *
  * 调用规范：
  * - Safety_Init()   : 上电初始化时调用一次
  * - Safety_Update() : 由主循环 100ms 任务调度调用
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SAFETY_MONITOR_H
#define __SAFETY_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "common_def.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  安全监控模块初始化
 * @note   上电时调用一次，清除内部状态
 * @retval None
 */
void Safety_Init(void);

/**
 * @brief  安全监控轮询更新
 * @note   由主循环 100ms 任务调度调用
 *         检测 A/B~J/K~M/O 类异常，直接调用 Alarm_SetError/ClearError
 * @retval None
 */
void Safety_Update(void);

/**
 * @brief  置位 N 类自检失败异常
 * @note   由 self_test 模块在自检失败时调用
 * @retval None
 */
void Safety_SetSelfTestError(void);

/**
 * @brief  清除 N 类自检失败异常
 * @note   由 self_test 模块在自检通过后调用
 * @retval None
 */
void Safety_ClearSelfTestError(void);

/**
 * @brief  打印当前报警状态（调试用）
 * @note   通过串口输出 Alarm 模块当前所有激活异常
 * @retval None
 */
void Safety_PrintStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __SAFETY_MONITOR_H */

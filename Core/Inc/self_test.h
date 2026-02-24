/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : self_test.h
  * @brief          : 系统自检模块头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本模块实现上电自检流程（非阻塞状态机）：
  *   LOGO 显示（2000ms）→ Step1 期望状态识别（500ms）→
  *   Step2 继电器纠错（800ms，含 500ms 脉冲等待）→
  *   Step3 接触器检查（500ms）→ Step4 温度安全检测（700ms）→
  *   PASS / FAIL（结果显示 1500ms）
  *
  * 期间 OLED 实时显示进度条（0%~100%）。
  * 结果通过 Safety_SetSelfTestError / Safety_ClearSelfTestError 通知安全模块。
  *
  * 调用规范：
  *   - SelfTest_Start()  : 所有模块初始化完成后调用一次
  *   - SelfTest_Update() : 由主循环每 20ms 调用（仅自检运行期间）
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SELF_TEST_H
#define __SELF_TEST_H

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
 * @brief 自检状态枚举
 */
typedef enum
{
    SELF_TEST_IDLE                  = 0, ///< 空闲（未开始或已结束）
    SELF_TEST_LOGO                  = 1, ///< LOGO 显示阶段
    SELF_TEST_STEP1_STATE_IDENTIFY  = 2, ///< Step1：K_EN 期望状态识别
    SELF_TEST_STEP2_RELAY_CHECK     = 3, ///< Step2：继电器 STA 纠错
    SELF_TEST_STEP3_SWITCH_CHECK    = 4, ///< Step3：接触器 SW_STA 检查
    SELF_TEST_STEP4_TEMP_CHECK      = 5, ///< Step4：NTC 过温检测
    SELF_TEST_PASS                  = 6, ///< 自检通过
    SELF_TEST_FAIL                  = 7  ///< 自检失败
} SelfTest_State_e;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  启动系统自检
 * @note   在所有模块初始化完成后调用一次，自动进入 LOGO→步骤→结果流程
 * @retval None
 */
void SelfTest_Start(void);

/**
 * @brief  查询自检是否仍在运行
 * @retval 1 = 运行中（LOGO/Step1~4/PASS/FAIL 等待期）；0 = 已结束
 */
uint8_t SelfTest_IsRunning(void);

/**
 * @brief  查询自检是否通过
 * @note   仅在 SelfTest_IsRunning() 返回 0 后有意义
 * @retval 1 = 通过；0 = 失败或尚未完成
 */
uint8_t SelfTest_IsPassed(void);

/**
 * @brief  自检状态机更新（非阻塞）
 * @note   由主循环每 20ms 调用，推进 LOGO→Step1~4→PASS/FAIL
 *         内部不调用 Relay_Update()，由主循环 20ms 任务单独驱动
 * @retval None
 */
void SelfTest_Update(void);

/**
 * @brief  打印自检最终结果（调试用）
 * @note   通过串口输出四步子结果及总判定
 * @retval None
 */
void SelfTest_PrintResult(void);

#ifdef __cplusplus
}
#endif

#endif /* __SELF_TEST_H */

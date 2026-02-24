/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : safety_monitor.c
  * @brief          : 安全监控模块实现
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 实现说明：
  * - Alarm 是唯一错误状态源，本模块不维护 active_errors 位图
  * - 每类异常在每次 Safety_Update() 中均独立判断并调用 Set 或 Clear
  * - B~J 类检测仅在所有通道空闲时执行，防止 500ms 脉冲期间误报
  * - 期望 STA 值直接由对应通道的 K_EN 电平推导，无需真值表
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "safety_monitor.h"
#include "alarm_output.h"
#include "relay_control.h"
#include "temperature.h"
#include <stdio.h>

/* Private types -------------------------------------------------------------*/

/**
 * @brief 模块内部状态（最小化，仅 2 个字段）
 */
typedef struct {
    bool             initialized; /* 初始化标志 */
    volatile uint8_t dc_fault;    /* 预留 ISR 标志，当前直接读 GPIO */
} Safety_Internal_t;

/* Private variables ---------------------------------------------------------*/

static Safety_Internal_t s_safety = {false, 0U};

/* Private function prototypes -----------------------------------------------*/

static void safety_check_relay_feedback(GPIO_PinState k1_en,
                                        GPIO_PinState k2_en,
                                        GPIO_PinState k3_en);
static void safety_check_temperature(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  安全监控模块初始化
 */
void Safety_Init(void)
{
    s_safety.initialized = true;
    s_safety.dc_fault    = 0U;
    printf("[Safety] Module initialized\r\n");
}

/**
 * @brief  安全监控轮询更新（100ms 调用一次）
 */
void Safety_Update(void)
{
    if (!s_safety.initialized)
    {
        return;
    }

    /* ① A类：使能冲突 —— ≥2路 K_EN 同时为低 */
    GPIO_PinState k1_en = HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin);
    GPIO_PinState k2_en = HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin);
    GPIO_PinState k3_en = HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin);

    uint8_t low_cnt = 0U;
    if (k1_en == GPIO_PIN_RESET) low_cnt++;
    if (k2_en == GPIO_PIN_RESET) low_cnt++;
    if (k3_en == GPIO_PIN_RESET) low_cnt++;

    if (low_cnt >= 2U)
    {
        Alarm_SetError(ERROR_TYPE_A);
    }
    else
    {
        Alarm_ClearError(ERROR_TYPE_A);
    }

    /* ② B~J类：继电器/接触器反馈
     *    跳过条件：
     *    a) 任意通道 FSM 处于 500ms 脉冲切换过渡期（Relay_IsChannelBusy）
     *    b) 任意通道 EN 引脚 ISR 已触发但 Relay_Update() 尚未处理
     *       （en_int.interrupt_flag != 0，即 K_EN 刚变化但 FSM 还未启动）
     *    覆盖竞争窗口：ISR 触发 → Relay_Update() 处理之间的空档期（≤20ms） */
    const Relay_Manager_t *mgr = Relay_GetManager();
    bool relay_stable =
        !Relay_IsChannelBusy(CHANNEL_1) &&
        !Relay_IsChannelBusy(CHANNEL_2) &&
        !Relay_IsChannelBusy(CHANNEL_3) &&
        (mgr->channels[0].en_int.interrupt_flag == 0U) &&
        (mgr->channels[1].en_int.interrupt_flag == 0U) &&
        (mgr->channels[2].en_int.interrupt_flag == 0U);

    if (relay_stable)
    {
        safety_check_relay_feedback(k1_en, k2_en, k3_en);
    }

    /* ③ K~M类：过温检测 */
    safety_check_temperature();

    /* ④ N类：由 self_test 模块主动调用 Safety_SetSelfTestError/
     *         Safety_ClearSelfTestError，此处不处理 */

    /* ⑤ O类：DC 电源掉电检测，直接读 GPIO
     *    DC_CTRL 为低电平 = 电源正常（外部电路主动拉低）
     *    DC_CTRL 为高电平 = 电源掉电（上拉电阻拉高，外部信号消失）*/
    if (HAL_GPIO_ReadPin(DC_CTRL_GPIO_Port, DC_CTRL_Pin) == GPIO_PIN_SET)
    {
        Alarm_SetError(ERROR_TYPE_O);
    }
    else
    {
        Alarm_ClearError(ERROR_TYPE_O);
    }
}

/**
 * @brief  置位 N 类自检失败异常
 */
void Safety_SetSelfTestError(void)
{
    Alarm_SetError(ERROR_TYPE_N);
}

/**
 * @brief  清除 N 类自检失败异常
 */
void Safety_ClearSelfTestError(void)
{
    Alarm_ClearError(ERROR_TYPE_N);
}

/**
 * @brief  打印当前报警状态（调试用）
 */
void Safety_PrintStatus(void)
{
    uint16_t flags = Alarm_GetErrorFlags();

    printf("[Safety] Active errors: 0x%04X", (unsigned int)flags);

    if (flags == (uint16_t)ERROR_TYPE_NONE)
    {
        printf(" (None)\r\n");
        return;
    }

    printf("\r\n");
    if (flags & (uint16_t)ERROR_TYPE_A) printf("  A - Enable Conflict\r\n");
    if (flags & (uint16_t)ERROR_TYPE_B) printf("  B - K1_1_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_C) printf("  C - K2_1_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_D) printf("  D - K3_1_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_E) printf("  E - K1_2_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_F) printf("  F - K2_2_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_G) printf("  G - K3_2_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_H) printf("  H - SW1_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_I) printf("  I - SW2_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_J) printf("  J - SW3_STA Error\r\n");
    if (flags & (uint16_t)ERROR_TYPE_K) printf("  K - NTC1 Overheat\r\n");
    if (flags & (uint16_t)ERROR_TYPE_L) printf("  L - NTC2 Overheat\r\n");
    if (flags & (uint16_t)ERROR_TYPE_M) printf("  M - NTC3 Overheat\r\n");
    if (flags & (uint16_t)ERROR_TYPE_N) printf("  N - Self-Test Failed\r\n");
    if (flags & (uint16_t)ERROR_TYPE_O) printf("  O - Power Error\r\n");
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  B~J 类：继电器与接触器 STA 反馈检测
 * @note   仅在所有通道空闲时调用
 *         期望值推导规则：K_EN 低电平（通道使能）→ STA 应为高电平（吸合）
 *                         K_EN 高电平（通道禁用）→ STA 应为低电平（断开）
 */
static void safety_check_relay_feedback(GPIO_PinState k1_en,
                                        GPIO_PinState k2_en,
                                        GPIO_PinState k3_en)
{
    /* 期望状态：K_EN = LOW → STA 期望 HIGH；K_EN = HIGH → STA 期望 LOW */
    GPIO_PinState exp1 = (k1_en == GPIO_PIN_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState exp2 = (k2_en == GPIO_PIN_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState exp3 = (k3_en == GPIO_PIN_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    /* B: CH1 继电器1 */
    if (HAL_GPIO_ReadPin(K1_1_STA_GPIO_Port, K1_1_STA_Pin) != exp1)
        Alarm_SetError(ERROR_TYPE_B);
    else
        Alarm_ClearError(ERROR_TYPE_B);

    /* C: CH2 继电器1 */
    if (HAL_GPIO_ReadPin(K2_1_STA_GPIO_Port, K2_1_STA_Pin) != exp2)
        Alarm_SetError(ERROR_TYPE_C);
    else
        Alarm_ClearError(ERROR_TYPE_C);

    /* D: CH3 继电器1 */
    if (HAL_GPIO_ReadPin(K3_1_STA_GPIO_Port, K3_1_STA_Pin) != exp3)
        Alarm_SetError(ERROR_TYPE_D);
    else
        Alarm_ClearError(ERROR_TYPE_D);

    /* E: CH1 继电器2 */
    if (HAL_GPIO_ReadPin(K1_2_STA_GPIO_Port, K1_2_STA_Pin) != exp1)
        Alarm_SetError(ERROR_TYPE_E);
    else
        Alarm_ClearError(ERROR_TYPE_E);

    /* F: CH2 继电器2 */
    if (HAL_GPIO_ReadPin(K2_2_STA_GPIO_Port, K2_2_STA_Pin) != exp2)
        Alarm_SetError(ERROR_TYPE_F);
    else
        Alarm_ClearError(ERROR_TYPE_F);

    /* G: CH3 继电器2 */
    if (HAL_GPIO_ReadPin(K3_2_STA_GPIO_Port, K3_2_STA_Pin) != exp3)
        Alarm_SetError(ERROR_TYPE_G);
    else
        Alarm_ClearError(ERROR_TYPE_G);

    /* H: CH1 接触器 */
    if (HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin) != exp1)
        Alarm_SetError(ERROR_TYPE_H);
    else
        Alarm_ClearError(ERROR_TYPE_H);

    /* I: CH2 接触器 */
    if (HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin) != exp2)
        Alarm_SetError(ERROR_TYPE_I);
    else
        Alarm_ClearError(ERROR_TYPE_I);

    /* J: CH3 接触器 */
    if (HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin) != exp3)
        Alarm_SetError(ERROR_TYPE_J);
    else
        Alarm_ClearError(ERROR_TYPE_J);
}

/**
 * @brief  K~M 类：三路 NTC 过温检测
 * @note   迟滞逻辑由 temperature 模块内部处理（含 2°C 回差）
 */
static void safety_check_temperature(void)
{
    if (Temperature_GetOverheatFlag(0U) == TEMP_STATUS_OVERHEAT)
        Alarm_SetError(ERROR_TYPE_K);
    else
        Alarm_ClearError(ERROR_TYPE_K);

    if (Temperature_GetOverheatFlag(1U) == TEMP_STATUS_OVERHEAT)
        Alarm_SetError(ERROR_TYPE_L);
    else
        Alarm_ClearError(ERROR_TYPE_L);

    if (Temperature_GetOverheatFlag(2U) == TEMP_STATUS_OVERHEAT)
        Alarm_SetError(ERROR_TYPE_M);
    else
        Alarm_ClearError(ERROR_TYPE_M);
}

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : relay_control.h
  * @brief          : 继电器控制模块头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-13
  ******************************************************************************
  * @attention
  *
 * 本模块实现三通道继电器控制功能：
 * 1. 三通道互锁逻辑（同一时间只能激活一个通道）
 * 2. 双路继电器控制（每通道2路：ON/OFF）
 * 3. 500ms负脉冲生成（非阻塞，低电平触发）
 * 4. 状态反馈检测（6路继电器 + 3路接触器）
 * 5. 防抖检测（50ms间隔×3次）
 * 6. K1_EN/K2_EN/K3_EN外部中断响应
 *
 * 磁保持继电器控制逻辑：
 * - 静态状态：所有控制引脚保持高电平
 * - 触发动作：输出500ms低电平脉冲
 * - 脉冲结束：恢复高电平，继电器保持状态（磁保持）
  *
  * 硬件连接：
  * 通道1：K1_1_ON/OFF (PC0/PC1), K1_2_ON/OFF (PA12/PA3)
  *        K1_1_STA (PC4), K1_2_STA (PB1), SW1_STA (PA8)
  *        K1_EN (PB9, 外部中断)
  *
  * 通道2：K2_1_ON/OFF (PC2/PC3), K2_2_ON/OFF (PA4/PA5)
  *        K2_1_STA (PC5), K2_2_STA (PB10), SW2_STA (PC9)
  *        K2_EN (PB8, 外部中断)
  *
  * 通道3：K3_1_ON/OFF (PC7/PC6), K3_2_ON/OFF (PD2/PA7)
  *        K3_1_STA (PB0), K3_2_STA (PB11), SW3_STA (PC8)
  *        K3_EN (PA15, 外部中断)
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RELAY_CONTROL_H
#define __RELAY_CONTROL_H

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
 * @brief 继电器操作类型枚举
 */
typedef enum
{
    RELAY_OP_NONE   = 0,  // 无操作
    RELAY_OP_OPEN   = 1,  // 打开（接通）
    RELAY_OP_CLOSE  = 2   // 关闭（断开）
} RelayOperation_e;

/**
 * @brief 继电器状态机枚举
 */
typedef enum
{
    RELAY_FSM_IDLE      = 0,  // 空闲状态
    RELAY_FSM_OPENING   = 1,  // 正在打开（500ms脉冲中）
    RELAY_FSM_CLOSING   = 2,  // 正在关闭（500ms脉冲中）
    RELAY_FSM_OPENED    = 3,  // 已打开
    RELAY_FSM_CLOSED    = 4,  // 已关闭
    RELAY_FSM_ERROR     = 5   // 错误状态（状态反馈异常）
} RelayFSM_e;

/**
 * @brief 单路继电器控制结构
 */
typedef struct {
    GPIO_TypeDef *on_port;       // ON引脚端口
    uint16_t on_pin;             // ON引脚号
    GPIO_TypeDef *off_port;      // OFF引脚端口
    uint16_t off_pin;            // OFF引脚号
    GPIO_TypeDef *sta_port;      // STA状态反馈引脚端口
    uint16_t sta_pin;            // STA状态反馈引脚号
    RelayFSM_e fsm_state;        // 状态机状态
    uint32_t pulse_start_time;   // 脉冲开始时间
    bool expected_state;         // 预期状态（true-ON, false-OFF）
    uint8_t debounce_count;      // 防抖计数器
} RelayUnit_t;

/**
 * @brief 通道控制结构
 */
typedef struct {
    RelayUnit_t relay1;          // 第1路继电器
    RelayUnit_t relay2;          // 第2路继电器
    GPIO_TypeDef *sw_sta_port;   // 接触器状态反馈引脚端口
    uint16_t sw_sta_pin;         // 接触器状态反馈引脚号
    GPIO_TypeDef *en_port;       // 使能引脚端口
    uint16_t en_pin;             // 使能引脚号
    bool is_active;              // 通道是否激活
    RelayOperation_e pending_op; // 待执行操作
} ChannelControl_t;

/**
 * @brief 继电器管理器数据结构
 */
typedef struct {
    ChannelControl_t channels[3];  // 3个通道控制结构
    Channel_e active_channel;      // 当前激活通道（CHANNEL_NONE表示全关闭）
    uint32_t last_update_time;     // 上次更新时间
    bool initialized;              // 初始化标志
} Relay_Manager_t;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief 继电器控制时间参数（单位：ms）
 */
#define RELAY_PULSE_WIDTH       500    // 继电器脉冲宽度：500ms
#define RELAY_DEBOUNCE_INTERVAL 50     // 防抖检测间隔：50ms
#define RELAY_DEBOUNCE_TIMES    3      // 防抖检测次数：3次

/**
 * @brief 状态反馈期望值（高电平=正常工作）
 */
#define RELAY_STA_EXPECTED_ON   GPIO_PIN_SET    // 继电器ON时，STA应为高电平
#define RELAY_STA_EXPECTED_OFF  GPIO_PIN_RESET  // 继电器OFF时，STA应为低电平

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief 继电器状态检查宏
 */
#define IS_RELAY_BUSY(unit)     ((unit)->fsm_state == RELAY_FSM_OPENING || \
                                 (unit)->fsm_state == RELAY_FSM_CLOSING)
#define IS_RELAY_OPENED(unit)   ((unit)->fsm_state == RELAY_FSM_OPENED)
#define IS_RELAY_CLOSED(unit)   ((unit)->fsm_state == RELAY_FSM_CLOSED)
#define IS_RELAY_ERROR(unit)    ((unit)->fsm_state == RELAY_FSM_ERROR)

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  继电器模块初始化
 * @note   初始化GPIO、状态机、关闭所有继电器
 * @retval None
 */
void Relay_Init(void);

/**
 * @brief  打开指定通道
 * @param  channel: 通道编号（CHANNEL_1/2/3）
 * @note   会自动关闭其他通道（互锁机制）
 *         发送500ms低电平脉冲到ON引脚（磁保持继电器）
 * @retval true-成功启动, false-失败（其他通道忙）
 */
bool Relay_OpenChannel(Channel_e channel);

/**
 * @brief  关闭指定通道
 * @param  channel: 通道编号（CHANNEL_1/2/3）
 * @note   发送500ms低电平脉冲到OFF引脚（磁保持继电器）
 * @retval true-成功启动, false-失败（通道忙）
 */
bool Relay_CloseChannel(Channel_e channel);

/**
 * @brief  关闭所有通道
 * @note   用于紧急停止或系统复位
 * @retval None
 */
void Relay_CloseAll(void);

/**
 * @brief  获取当前激活通道
 * @retval 当前激活的通道编号（CHANNEL_NONE表示全关闭）
 */
Channel_e Relay_GetActiveChannel(void);

/**
 * @brief  检查通道是否忙碌
 * @param  channel: 通道编号
 * @retval true-忙碌（正在执行操作）, false-空闲
 */
bool Relay_IsChannelBusy(Channel_e channel);

/**
 * @brief  检查通道状态反馈是否正常
 * @param  channel: 通道编号
 * @retval true-正常, false-异常
 */
bool Relay_CheckChannelFeedback(Channel_e channel);

/**
 * @brief  更新继电器状态（非阻塞）
 * @note   定时调用此函数更新状态机、检测状态反馈
 *         建议调用频率：20-50Hz（20-50ms）
 * @retval None
 */
void Relay_Update(void);

/**
 * @brief  打印继电器状态（调试用）
 * @note   通过串口输出当前通道状态、继电器状态等信息
 * @retval None
 */
void Relay_PrintStatus(void);

/**
 * @brief  K1_EN中断回调（外部调用）
 * @note   由stm32f1xx_it.c中的HAL_GPIO_EXTI_Callback调用
 * @retval None
 */
void Relay_K1_EN_ISR(void);

/**
 * @brief  K2_EN中断回调（外部调用）
 * @note   由stm32f1xx_it.c中的HAL_GPIO_EXTI_Callback调用
 * @retval None
 */
void Relay_K2_EN_ISR(void);

/**
 * @brief  K3_EN中断回调（外部调用）
 * @note   由stm32f1xx_it.c中的HAL_GPIO_EXTI_Callback调用
 * @retval None
 */
void Relay_K3_EN_ISR(void);

/**
 * @brief  获取继电器管理器指针（调试用）
 * @retval 继电器管理器数据结构指针
 */
const Relay_Manager_t* Relay_GetManager(void);

#ifdef __cplusplus
}
#endif

#endif /* __RELAY_CONTROL_H */

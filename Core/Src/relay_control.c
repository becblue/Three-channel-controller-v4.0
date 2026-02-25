/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : relay_control.c
  * @brief          : 继电器控制模块实�?
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-13
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "relay_control.h"
#include "data_logger.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifndef EN_PIN_VERIFY_DELAY_MS
#define EN_PIN_VERIFY_DELAY_MS  100    // EN pin verify delay: 100ms (increased for better stability)
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// 继电器管理器实例
static Relay_Manager_t relay_manager = {0};

/* Private function prototypes -----------------------------------------------*/
static void relay_start_pulse(RelayUnit_t *unit, bool turn_on);
static void relay_stop_pulse(RelayUnit_t *unit);
static void relay_update_fsm(RelayUnit_t *unit);
static bool relay_check_feedback(RelayUnit_t *unit);
static GPIO_PinState relay_read_sta(RelayUnit_t *unit);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  继电器模块初始化
 */
void Relay_Init(void)
{
    // 清零结构�?
    memset(&relay_manager, 0, sizeof(Relay_Manager_t));
    
    // 初始化通道1的GPIO映射
    relay_manager.channels[0].relay1.on_port = K1_1_ON_GPIO_Port;
    relay_manager.channels[0].relay1.on_pin = K1_1_ON_Pin;
    relay_manager.channels[0].relay1.off_port = K1_1_OFF_GPIO_Port;
    relay_manager.channels[0].relay1.off_pin = K1_1_OFF_Pin;
    relay_manager.channels[0].relay1.sta_port = K1_1_STA_GPIO_Port;
    relay_manager.channels[0].relay1.sta_pin = K1_1_STA_Pin;
    
    relay_manager.channels[0].relay2.on_port = K1_2_ON_GPIO_Port;
    relay_manager.channels[0].relay2.on_pin = K1_2_ON_Pin;
    relay_manager.channels[0].relay2.off_port = K1_2_OFF_GPIO_Port;
    relay_manager.channels[0].relay2.off_pin = K1_2_OFF_Pin;
    relay_manager.channels[0].relay2.sta_port = K1_2_STA_GPIO_Port;
    relay_manager.channels[0].relay2.sta_pin = K1_2_STA_Pin;
    
    relay_manager.channels[0].sw_sta_port = SW1_STA_GPIO_Port;
    relay_manager.channels[0].sw_sta_pin = SW1_STA_Pin;
    relay_manager.channels[0].en_port = K1_EN_GPIO_Port;
    relay_manager.channels[0].en_pin = K1_EN_Pin;
    
    // 初始化通道2的GPIO映射
    relay_manager.channels[1].relay1.on_port = K2_1_ON_GPIO_Port;
    relay_manager.channels[1].relay1.on_pin = K2_1_ON_Pin;
    relay_manager.channels[1].relay1.off_port = K2_1_OFF_GPIO_Port;
    relay_manager.channels[1].relay1.off_pin = K2_1_OFF_Pin;
    relay_manager.channels[1].relay1.sta_port = K2_1_STA_GPIO_Port;
    relay_manager.channels[1].relay1.sta_pin = K2_1_STA_Pin;
    
    relay_manager.channels[1].relay2.on_port = K2_2_ON_GPIO_Port;
    relay_manager.channels[1].relay2.on_pin = K2_2_ON_Pin;
    relay_manager.channels[1].relay2.off_port = K2_2_OFF_GPIO_Port;
    relay_manager.channels[1].relay2.off_pin = K2_2_OFF_Pin;
    relay_manager.channels[1].relay2.sta_port = K2_2_STA_GPIO_Port;
    relay_manager.channels[1].relay2.sta_pin = K2_2_STA_Pin;
    
    relay_manager.channels[1].sw_sta_port = SW2_STA_GPIO_Port;
    relay_manager.channels[1].sw_sta_pin = SW2_STA_Pin;
    relay_manager.channels[1].en_port = K2_EN_GPIO_Port;
    relay_manager.channels[1].en_pin = K2_EN_Pin;
    
    // 初始化通道3的GPIO映射
    relay_manager.channels[2].relay1.on_port = K3_1_ON_GPIO_Port;
    relay_manager.channels[2].relay1.on_pin = K3_1_ON_Pin;
    relay_manager.channels[2].relay1.off_port = K3_1_OFF_GPIO_Port;
    relay_manager.channels[2].relay1.off_pin = K3_1_OFF_Pin;
    relay_manager.channels[2].relay1.sta_port = K3_1_STA_GPIO_Port;
    relay_manager.channels[2].relay1.sta_pin = K3_1_STA_Pin;
    
    relay_manager.channels[2].relay2.on_port = K3_2_ON_GPIO_Port;
    relay_manager.channels[2].relay2.on_pin = K3_2_ON_Pin;
    relay_manager.channels[2].relay2.off_port = K3_2_OFF_GPIO_Port;
    relay_manager.channels[2].relay2.off_pin = K3_2_OFF_Pin;
    relay_manager.channels[2].relay2.sta_port = K3_2_STA_GPIO_Port;
    relay_manager.channels[2].relay2.sta_pin = K3_2_STA_Pin;
    
    relay_manager.channels[2].sw_sta_port = SW3_STA_GPIO_Port;
    relay_manager.channels[2].sw_sta_pin = SW3_STA_Pin;
    relay_manager.channels[2].en_port = K3_EN_GPIO_Port;
    relay_manager.channels[2].en_pin = K3_EN_Pin;
    
    // 初始化所有继电器为关闭状�?
    for (int i = 0; i < 3; i++)
    {
        relay_manager.channels[i].relay1.fsm_state = RELAY_FSM_CLOSED;
        relay_manager.channels[i].relay1.expected_state = false;
        relay_manager.channels[i].relay2.fsm_state = RELAY_FSM_CLOSED;
        relay_manager.channels[i].relay2.expected_state = false;
        relay_manager.channels[i].is_active = false;
        relay_manager.channels[i].pending_op = RELAY_OP_NONE;
        
        // 磁保持继电器：所有控制引脚初始化为高电平（静态状态）
        HAL_GPIO_WritePin(relay_manager.channels[i].relay1.on_port, 
                         relay_manager.channels[i].relay1.on_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(relay_manager.channels[i].relay1.off_port, 
                         relay_manager.channels[i].relay1.off_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(relay_manager.channels[i].relay2.on_port, 
                         relay_manager.channels[i].relay2.on_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(relay_manager.channels[i].relay2.off_port, 
                         relay_manager.channels[i].relay2.off_pin, GPIO_PIN_SET);
    }
    
    relay_manager.active_channel = CHANNEL_NONE;
    relay_manager.last_update_time = HAL_GetTick();
    // 初始化EN引脚中断管理结构
    for (int i = 0; i < 3; i++)
    {
        relay_manager.channels[i].en_int.interrupt_flag = 0;
        relay_manager.channels[i].en_int.interrupt_time = 0;
        relay_manager.channels[i].en_int.verify_step = 0;
        
        // 读取并记录EN引脚的初始状�?
        GPIO_TypeDef *en_port = relay_manager.channels[i].en_port;
        uint16_t en_pin = relay_manager.channels[i].en_pin;
        relay_manager.channels[i].en_int.last_pin_state = HAL_GPIO_ReadPin(en_port, en_pin);
        relay_manager.channels[i].en_int.pending_pin_state = GPIO_PIN_RESET;
    }
    
    relay_manager.initialized = true;
    
    printf("[Relay] Module initialized\r\n");
    printf("[Relay] All channels closed (interlock enabled)\r\n");
    printf("[Relay] Pulse width: %dms\r\n", RELAY_PULSE_WIDTH);
    printf("[Relay] Debounce: %dms x %d times\r\n", 
           RELAY_DEBOUNCE_INTERVAL, RELAY_DEBOUNCE_TIMES);
}

/**
 * @brief  打开指定通道
 */
bool Relay_OpenChannel(Channel_e channel)
{
    if (!relay_manager.initialized) return false;
    if (channel < CHANNEL_1 || channel > CHANNEL_3) return false;
    
    uint8_t ch_idx = channel - 1;
    
    // 检查互锁：如果其他通道激活，禁止操作
    if (relay_manager.active_channel != CHANNEL_NONE && 
        relay_manager.active_channel != channel)
    {
        printf("[Relay] Interlock: CH%d is active, cannot open CH%d\r\n", 
               relay_manager.active_channel, channel);
        return false;
    }
    
    // 检查通道是否忙碌
    if (IS_RELAY_BUSY(&relay_manager.channels[ch_idx].relay1) ||
        IS_RELAY_BUSY(&relay_manager.channels[ch_idx].relay2))
    {
        printf("[Relay] CH%d is busy\r\n", channel);
        return false;
    }
    
    printf("[Relay] Opening CH%d...\r\n", channel);
    
    // 启动两路继电器的ON脉冲
    relay_start_pulse(&relay_manager.channels[ch_idx].relay1, true);
    relay_start_pulse(&relay_manager.channels[ch_idx].relay2, true);
    
    // 更新通道状�?
    relay_manager.channels[ch_idx].is_active = true;
    relay_manager.channels[ch_idx].pending_op = RELAY_OP_OPEN;
    relay_manager.active_channel = channel;

    /* Log channel open action to external Flash */
    DataLogger_WriteChannelAction((uint8_t)channel, 1);
    
    return true;
}

/**
 * @brief  关闭指定通道
 */
bool Relay_CloseChannel(Channel_e channel)
{
    if (!relay_manager.initialized) return false;
    if (channel < CHANNEL_1 || channel > CHANNEL_3) return false;
    
    uint8_t ch_idx = channel - 1;
    
    // 检查通道是否忙碌
    if (IS_RELAY_BUSY(&relay_manager.channels[ch_idx].relay1) ||
        IS_RELAY_BUSY(&relay_manager.channels[ch_idx].relay2))
    {
        printf("[Relay] CH%d is busy\r\n", channel);
        return false;
    }
    
    printf("[Relay] Closing CH%d...\r\n", channel);
    
    // 启动两路继电器的OFF脉冲
    relay_start_pulse(&relay_manager.channels[ch_idx].relay1, false);
    relay_start_pulse(&relay_manager.channels[ch_idx].relay2, false);
    
    // 更新通道状�?
    relay_manager.channels[ch_idx].is_active = false;
    relay_manager.channels[ch_idx].pending_op = RELAY_OP_CLOSE;

    /* Log channel close action to external Flash */
    DataLogger_WriteChannelAction((uint8_t)channel, 0);
    
    // 如果是当前激活通道，清除激活标�?
    if (relay_manager.active_channel == channel)
    {
        relay_manager.active_channel = CHANNEL_NONE;
    }
    
    return true;
}

/**
 * @brief  关闭所有通道
 */
void Relay_CloseAll(void)
{
    printf("[Relay] Closing all channels...\r\n");
    
    for (int i = 0; i < 3; i++)
    {
        if (relay_manager.channels[i].is_active)
        {
            Relay_CloseChannel((Channel_e)(i + 1));
        }
    }
}

/**
 * @brief  上电复位：强制对三路全部发�?OFF 脉冲
 * @note   �?Relay_CloseAll() 的区别：
 *         - 不检�?is_active 标志，磁保持继电器上电后状态未知，全部强制关断
 *         - 用于自检 LOGO 阶段�?000ms），脉冲 500ms 内完�?
 *         - 不影�?Step2 纠错逻辑，两者互�?
 */
void Relay_ForceCloseAll(void)
{
    printf("[Relay] Force closing all channels (power-on reset)...\r\n");

    for (uint8_t i = 0U; i < 3U; i++)
    {
        /* 若通道正忙（上一次脉冲未完成），跳过，避免引脚状态冲�?*/
        if (IS_RELAY_BUSY(&relay_manager.channels[i].relay1) ||
            IS_RELAY_BUSY(&relay_manager.channels[i].relay2))
        {
            printf("[Relay] CH%d busy, skip force close\r\n", (int)(i + 1U));
            continue;
        }

        /* 直接发�?OFF 脉冲，无�?is_active */
        relay_start_pulse(&relay_manager.channels[i].relay1, false);
        relay_start_pulse(&relay_manager.channels[i].relay2, false);
        relay_manager.channels[i].is_active  = false;
        relay_manager.channels[i].pending_op = RELAY_OP_NONE;

        printf("[Relay] CH%d force OFF pulse sent\r\n", (int)(i + 1U));
    }

    relay_manager.active_channel = CHANNEL_NONE;
}

/**
 * @brief  获取当前激活通道
 */
Channel_e Relay_GetActiveChannel(void)
{
    return relay_manager.active_channel;
}

/**
 * @brief  检查通道是否忙碌
 */
bool Relay_IsChannelBusy(Channel_e channel)
{
    if (channel < CHANNEL_1 || channel > CHANNEL_3) return false;
    
    uint8_t ch_idx = channel - 1;
    return (IS_RELAY_BUSY(&relay_manager.channels[ch_idx].relay1) ||
            IS_RELAY_BUSY(&relay_manager.channels[ch_idx].relay2));
}

/**
 * @brief  检查通道状态反馈是否正�?
 */
bool Relay_CheckChannelFeedback(Channel_e channel)
{
    if (channel < CHANNEL_1 || channel > CHANNEL_3) return false;
    
    uint8_t ch_idx = channel - 1;
    
    // 检查两路继电器的状态反�?
    bool relay1_ok = relay_check_feedback(&relay_manager.channels[ch_idx].relay1);
    bool relay2_ok = relay_check_feedback(&relay_manager.channels[ch_idx].relay2);
    
    return (relay1_ok && relay2_ok);
}

/**
 * @brief  更新继电器状态（非阻塞）
 */
void Relay_Update(void)
{
    if (!relay_manager.initialized) return;
    
    // Process EN pin interrupt verification (NEW APPROACH)
    for (int i = 0; i < 3; i++)
    {
        EN_InterruptManager_t *en_int = &relay_manager.channels[i].en_int;
        GPIO_TypeDef *en_port = relay_manager.channels[i].en_port;
        uint16_t en_pin = relay_manager.channels[i].en_pin;
        
        // Step 0: Check interrupt flag
        if (en_int->interrupt_flag && en_int->verify_step == 0)
        {
            // First reading after interrupt
            en_int->pending_pin_state = HAL_GPIO_ReadPin(en_port, en_pin);
            en_int->verify_step = 1;
            en_int->interrupt_time = HAL_GetTick(); // Start verification timer
        }
        
        // Step 1: Wait for stabilization period, then verify
        else if (en_int->verify_step == 1)
        {
            uint32_t current_time = HAL_GetTick();
            
            // Check if delay period has elapsed (non-blocking)
            if ((current_time - en_int->interrupt_time) >= EN_PIN_VERIFY_DELAY_MS)
            {
                // Second reading after delay
                GPIO_PinState current_state = HAL_GPIO_ReadPin(en_port, en_pin);
                
                // Verification 1: Are two readings consistent?
                if (current_state == en_int->pending_pin_state)
                {
                    // Verification 2: Did the state actually change? (KEY!)
                    if (current_state != en_int->last_pin_state)
                    {
                        // State changed - this is a valid trigger
                        if (current_state == GPIO_PIN_RESET)
                        {
                            /* EN 变低：外部正常使能，打开通道 */
                            relay_manager.channels[i].pending_op = RELAY_OP_OPEN;
                            printf("[ISR] K%d_EN LOW -> opening CH%d\r\n", i+1, i+1);
                        }
                        else
                        {
                            /* EN 变高：先判断 DC 电源是否存在
                             *   DC_CTRL = LOW  �?外部主动拉低，电源正常，属于正常关断指令
                             *   DC_CTRL = HIGH �?上拉拉高，外部信号消失，属于异常掉电
                             *                    不操作继电器，O 类报警由 Safety_Update() 自动处理 */
                            if (HAL_GPIO_ReadPin(DC_CTRL_GPIO_Port, DC_CTRL_Pin) == GPIO_PIN_RESET)
                            {
                                relay_manager.channels[i].pending_op = RELAY_OP_CLOSE;
                                printf("[ISR] K%d_EN HIGH + DC OK -> closing CH%d\r\n", i+1, i+1);
                            }
                            else
                            {
                                /* 异常掉电：保持继电器当前状态，仅输出报�?*/
                                printf("[ISR] K%d_EN HIGH + DC FAIL -> power loss detected, CH%d unchanged\r\n", i+1, i+1);
                            }
                        }
                        
                        // Update last state
                        en_int->last_pin_state = current_state;
                    }
                    // else: State did not change, ignore (FILTERS FALSE TRIGGERS!)
                }
                // else: Two readings inconsistent, ignore (noise)
                
                // Reset verification process
                en_int->interrupt_flag = 0;
                en_int->verify_step = 0;
            }
            // else: Still waiting for delay period, do nothing (non-blocking)
        }
    }
    
    // Process pending operations
    for (int i = 0; i < 3; i++)
    {
        if (relay_manager.channels[i].pending_op != RELAY_OP_NONE)
        {
            RelayOperation_e op = relay_manager.channels[i].pending_op;
            relay_manager.channels[i].pending_op = RELAY_OP_NONE;
            
            // Execute operation
            if (op == RELAY_OP_OPEN)
            {
                if (i == 0) Relay_OpenChannel(CHANNEL_1);
                else if (i == 1) Relay_OpenChannel(CHANNEL_2);
                else if (i == 2) Relay_OpenChannel(CHANNEL_3);
            }
            else if (op == RELAY_OP_CLOSE)
            {
                if (i == 0) Relay_CloseChannel(CHANNEL_1);
                else if (i == 1) Relay_CloseChannel(CHANNEL_2);
                else if (i == 2) Relay_CloseChannel(CHANNEL_3);
            }
        }
    }
    
    // Update all relay state machines
    for (int i = 0; i < 3; i++)
    {
        relay_update_fsm(&relay_manager.channels[i].relay1);
        relay_update_fsm(&relay_manager.channels[i].relay2);
    }
    
    relay_manager.last_update_time = HAL_GetTick();
}

/**
 * @brief  打印继电器状态（调试用）
 */
void Relay_PrintStatus(void)
{
    printf("\r\n========================================\r\n");
    printf("          Relay Status\r\n");
    printf("========================================\r\n");
    
    printf("Active Channel: %s\r\n", CommonDef_GetChannelString(relay_manager.active_channel));
    printf("\r\n");
    
    for (int i = 0; i < 3; i++)
    {
        printf("Channel %d:\r\n", i+1);
        
        // 继电�?状�?
        printf("  Relay1: ");
        switch (relay_manager.channels[i].relay1.fsm_state)
        {
            case RELAY_FSM_IDLE:    printf("IDLE"); break;
            case RELAY_FSM_OPENING: printf("OPENING"); break;
            case RELAY_FSM_CLOSING: printf("CLOSING"); break;
            case RELAY_FSM_OPENED:  printf("OPENED"); break;
            case RELAY_FSM_CLOSED:  printf("CLOSED"); break;
            case RELAY_FSM_ERROR:   printf("ERROR"); break;
            default:                printf("UNKNOWN"); break;
        }
        printf(" | STA=%d\r\n", relay_read_sta(&relay_manager.channels[i].relay1));
        
        // 继电�?状�?
        printf("  Relay2: ");
        switch (relay_manager.channels[i].relay2.fsm_state)
        {
            case RELAY_FSM_IDLE:    printf("IDLE"); break;
            case RELAY_FSM_OPENING: printf("OPENING"); break;
            case RELAY_FSM_CLOSING: printf("CLOSING"); break;
            case RELAY_FSM_OPENED:  printf("OPENED"); break;
            case RELAY_FSM_CLOSED:  printf("CLOSED"); break;
            case RELAY_FSM_ERROR:   printf("ERROR"); break;
            default:                printf("UNKNOWN"); break;
        }
        printf(" | STA=%d\r\n", relay_read_sta(&relay_manager.channels[i].relay2));
        
        // 接触器状�?
        GPIO_PinState sw_state = HAL_GPIO_ReadPin(relay_manager.channels[i].sw_sta_port, 
                                                   relay_manager.channels[i].sw_sta_pin);
        printf("  Switch: STA=%d\r\n", sw_state);
        printf("\r\n");
    }
    
    printf("========================================\r\n\r\n");
}

/**
 * @brief  K1_EN中断回调
 */
void Relay_K1_EN_ISR(void)
{
    // Only set interrupt flag - completely non-blocking
    relay_manager.channels[0].en_int.interrupt_flag = 1;
    relay_manager.channels[0].en_int.interrupt_time = HAL_GetTick();
    // Done! < 1us execution time
}

/**
 * @brief  K2_EN中断回调
 */
void Relay_K2_EN_ISR(void)
{
    // Only set interrupt flag - completely non-blocking
    relay_manager.channels[1].en_int.interrupt_flag = 1;
    relay_manager.channels[1].en_int.interrupt_time = HAL_GetTick();
    // Done! < 1us execution time
}

/**
 * @brief  K3_EN中断回调
 */
void Relay_K3_EN_ISR(void)
{
    // Only set interrupt flag - completely non-blocking
    relay_manager.channels[2].en_int.interrupt_flag = 1;
    relay_manager.channels[2].en_int.interrupt_time = HAL_GetTick();
    // Done! < 1us execution time
}

/**
 * @brief  获取继电器管理器指针（调试用�?
 */
const Relay_Manager_t* Relay_GetManager(void)
{
    return &relay_manager;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  启动继电器脉�?
 * @param  unit: 继电器单元指�?
 * @param  turn_on: true-打开（ON脉冲），false-关闭（OFF脉冲�?
 * @note   磁保持继电器：静态高电平，触发时输出500ms低电平脉�?
 */
static void relay_start_pulse(RelayUnit_t *unit, bool turn_on)
{
    if (turn_on)
    {
        // 输出ON脉冲（低电平触发�?
        // ON引脚输出低电平，OFF引脚保持高电�?
        HAL_GPIO_WritePin(unit->on_port, unit->on_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(unit->off_port, unit->off_pin, GPIO_PIN_SET);
        unit->fsm_state = RELAY_FSM_OPENING;
        unit->expected_state = true;
    }
    else
    {
        // 输出OFF脉冲（低电平触发�?
        // OFF引脚输出低电平，ON引脚保持高电�?
        HAL_GPIO_WritePin(unit->on_port, unit->on_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(unit->off_port, unit->off_pin, GPIO_PIN_RESET);
        unit->fsm_state = RELAY_FSM_CLOSING;
        unit->expected_state = false;
    }
    
    // 记录脉冲开始时�?
    unit->pulse_start_time = HAL_GetTick();
    unit->debounce_count = 0;
}

/**
 * @brief  停止继电器脉�?
 * @param  unit: 继电器单元指�?
 * @note   磁保持继电器：脉冲结束后，恢复所有引脚为高电平（静态状态）
 */
static void relay_stop_pulse(RelayUnit_t *unit)
{
    // 恢复所有控制引脚为高电平（静态状态）
    HAL_GPIO_WritePin(unit->on_port, unit->on_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(unit->off_port, unit->off_pin, GPIO_PIN_SET);
}

/**
 * @brief  更新继电器状态机
 * @param  unit: 继电器单元指�?
 */
static void relay_update_fsm(RelayUnit_t *unit)
{
    uint32_t current_time = HAL_GetTick();
    
    switch (unit->fsm_state)
    {
        case RELAY_FSM_OPENING:
        case RELAY_FSM_CLOSING:
            // 检查脉冲是否完成（500ms�?
            if ((current_time - unit->pulse_start_time) >= RELAY_PULSE_WIDTH)
            {
                // 停止脉冲
                relay_stop_pulse(unit);
                
                // 等待状态反馈稳�?
                unit->debounce_count = 0;
                
                // 切换到对应的完成状�?
                if (unit->fsm_state == RELAY_FSM_OPENING)
                    unit->fsm_state = RELAY_FSM_OPENED;
                else
                    unit->fsm_state = RELAY_FSM_CLOSED;
            }
            break;
            
        case RELAY_FSM_OPENED:
        case RELAY_FSM_CLOSED:
            // 检查状态反馈（带防抖）
            if (!relay_check_feedback(unit))
            {
                // 状态反馈异�?
                unit->fsm_state = RELAY_FSM_ERROR;
                printf("[Relay] Feedback error detected!\r\n");
            }
            break;
            
        case RELAY_FSM_ERROR:
            // 错误状态，等待外部清除或重新初始化
            break;
            
        case RELAY_FSM_IDLE:
        default:
            break;
    }
}

/**
 * @brief  检查状态反馈（带防抖）
 * @param  unit: 继电器单元指�?
 * @retval true-反馈正常, false-反馈异常
 */
static bool relay_check_feedback(RelayUnit_t *unit)
{
    // 读取STA引脚状�?
    GPIO_PinState sta_state = relay_read_sta(unit);
    
    // 判断是否符合预期
    GPIO_PinState expected = unit->expected_state ? RELAY_STA_EXPECTED_ON : RELAY_STA_EXPECTED_OFF;
    
    if (sta_state == expected)
    {
        unit->debounce_count = 0;  // 状态正常，清零计数�?
        return true;
    }
    else
    {
        // 状态异常，防抖计数
        // TODO: 实现完整的防抖逻辑
        // static uint32_t last_debounce_time[3][2] = {0};
        // uint32_t current_time = HAL_GetTick();
        
        // 简化处理：直接返回异常（实际应用中可实现完整防抖）
        return false;
    }
}

/**
 * @brief  读取继电器状态反馈引�?
 * @param  unit: 继电器单元指�?
 * @retval GPIO引脚状�?
 */
static GPIO_PinState relay_read_sta(RelayUnit_t *unit)
{
    return HAL_GPIO_ReadPin(unit->sta_port, unit->sta_pin);
}

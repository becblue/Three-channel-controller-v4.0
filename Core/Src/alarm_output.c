/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : alarm_output.c
  * @brief          : 报警输出控制模块实现
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "alarm_output.h"
#include "data_logger.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// 报警管理器实�?
static Alarm_Manager_t alarm_manager = {0};

/* Private function prototypes -----------------------------------------------*/
static void alarm_update_beep_mode(void);
static void alarm_update_gpio(void);
static void alarm_beep_pulse_handler(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  报警模块初始�?
 */
void Alarm_Init(void)
{
    // 清零结构�?
    memset(&alarm_manager, 0, sizeof(Alarm_Manager_t));
    
    // 初始化GPIO状态（高电�?无报警）
    HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
    
    // 初始化参�?
    alarm_manager.error_flags = ERROR_TYPE_NONE;
    alarm_manager.beep_mode = BEEP_MODE_OFF;
    alarm_manager.beep_state = false;
    alarm_manager.alarm_active = false;
    alarm_manager.beep_timer = 0;
    alarm_manager.initialized = true;
    
    printf("[Alarm] Module initialized\r\n");
    printf("[Alarm] ALARM pin: PB4 (HIGH=Normal)\r\n");
    printf("[Alarm] BEEP pin:  PB3 (HIGH=Silent)\r\n");
}

/**
 * @brief  设置异常标志
 */
void Alarm_SetError(ErrorType_e error_type)
{
    if (!alarm_manager.initialized) return;
    
    // 如果是新异常，记录日�?
    if ((alarm_manager.error_flags & error_type) == 0)
    {
        printf("[Alarm] Set error: 0x%04X (%s)\r\n", 
               error_type, CommonDef_GetErrorString(error_type));
    }
    
    // 设置异常标志（位或操作）
    alarm_manager.error_flags |= error_type;

    /* Log alarm set event to external Flash */
    DataLogger_WriteAlarm((uint8_t)error_type, 1);
    
    // 更新蜂鸣器模�?
    alarm_update_beep_mode();
    
    // 立即更新GPIO状�?
    alarm_update_gpio();
}

/**
 * @brief  清除异常标志
 */
void Alarm_ClearError(ErrorType_e error_type)
{
    if (!alarm_manager.initialized) return;
    
    // 如果异常存在，记录日�?
    if ((alarm_manager.error_flags & error_type) != 0)
    {
        printf("[Alarm] Clear error: 0x%04X (%s)\r\n", 
               error_type, CommonDef_GetErrorString(error_type));
    }
    
    // 清除异常标志（位与非操作�?
    alarm_manager.error_flags &= ~error_type;

    /* Log alarm clear event to external Flash */
    DataLogger_WriteAlarm((uint8_t)error_type, 0);
    
    // 更新蜂鸣器模�?
    alarm_update_beep_mode();
    
    // 立即更新GPIO状�?
    alarm_update_gpio();
}

/**
 * @brief  检查是否有任何异常
 */
bool Alarm_HasError(void)
{
    return ALARM_HAS_ERROR(alarm_manager.error_flags);
}

/**
 * @brief  获取当前异常标志位图
 */
uint16_t Alarm_GetErrorFlags(void)
{
    return alarm_manager.error_flags;
}

/**
 * @brief  获取当前蜂鸣器模�?
 */
BeepMode_e Alarm_GetBeepMode(void)
{
    return alarm_manager.beep_mode;
}

/**
 * @brief  更新报警状态（非阻塞）
 */
void Alarm_Update(void)
{
    if (!alarm_manager.initialized) return;
    
    // 处理蜂鸣器脉冲逻辑
    alarm_beep_pulse_handler();
    
    // 更新GPIO状�?
    alarm_update_gpio();
}

/**
 * @brief  打印报警状态（调试用）
 */
void Alarm_PrintStatus(void)
{
    printf("\r\n========================================\r\n");
    printf("          Alarm Status\r\n");
    printf("========================================\r\n");
    
    // Error flags
    printf("Error Flags: 0x%04X\r\n", alarm_manager.error_flags);
    if (alarm_manager.error_flags != ERROR_TYPE_NONE)
    {
        printf("  Active errors:\r\n");
        for (int i = 0; i < 15; i++)
        {
            uint16_t err = (1 << i);
            if (alarm_manager.error_flags & err)
            {
                printf("    - %s\r\n", CommonDef_GetErrorString((ErrorType_e)err));
            }
        }
    }
    else
    {
        printf("  No errors\r\n");
    }
    
    // Beep mode
    printf("\r\nBeep Mode: ");
    switch (alarm_manager.beep_mode)
    {
        case BEEP_MODE_OFF:
            printf("OFF\r\n");
            break;
        case BEEP_MODE_CONTINUOUS:
            printf("Continuous (Temp error)\r\n");
            break;
        case BEEP_MODE_PULSE_50MS:
            printf("50ms pulse (Feedback error)\r\n");
            break;
        case BEEP_MODE_PULSE_1S:
            printf("1s pulse (Conflict/Self-test error)\r\n");
            break;
        default:
            printf("Unknown\r\n");
            break;
    }
    
    // GPIO status
    printf("\r\nGPIO Status:\r\n");
    printf("  ALARM: %s\r\n", alarm_manager.alarm_active ? "LOW (Active)" : "HIGH (Normal)");
    printf("  BEEP:  %s\r\n", alarm_manager.beep_state ? "Beeping" : "Silent");
    
    printf("========================================\r\n\r\n");
}

/**
 * @brief  清除所有异常标志（强制复位�?
 */
void Alarm_ClearAll(void)
{
    printf("[Alarm] Clear all errors\r\n");
    alarm_manager.error_flags = ERROR_TYPE_NONE;
    alarm_manager.beep_mode = BEEP_MODE_OFF;
    alarm_manager.beep_state = false;
    alarm_manager.alarm_active = false;
    alarm_manager.beep_timer = 0;
    
    // 关闭所有报警输�?
    HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
}

/**
 * @brief  获取报警管理器指针（调试用）
 */
const Alarm_Manager_t* Alarm_GetManager(void)
{
    return &alarm_manager;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  更新蜂鸣器模式（根据异常优先级）
 * @note   优先级：温度异常 > 状态反馈异�?> 冲突/自检异常
 */
static void alarm_update_beep_mode(void)
{
    BeepMode_e old_mode = alarm_manager.beep_mode;
    
    // 无异�?- 关闭蜂鸣�?
    if (alarm_manager.error_flags == ERROR_TYPE_NONE)
    {
        alarm_manager.beep_mode = BEEP_MODE_OFF;
    }
    // 温度异常（K~M�? 最高优先级，持续响
    else if (ALARM_HAS_TEMP_ERROR(alarm_manager.error_flags))
    {
        alarm_manager.beep_mode = BEEP_MODE_CONTINUOUS;
    }
    // 状态反馈异常（B~J�? 50ms脉冲
    else if (ALARM_HAS_FEEDBACK_ERROR(alarm_manager.error_flags))
    {
        alarm_manager.beep_mode = BEEP_MODE_PULSE_50MS;
    }
    // 冲突/自检异常（A、N、O�? 1秒脉�?
    else if (ALARM_HAS_CONFLICT_ERROR(alarm_manager.error_flags))
    {
        alarm_manager.beep_mode = BEEP_MODE_PULSE_1S;
    }
    
    // 模式切换时重置定时器
    if (old_mode != alarm_manager.beep_mode)
    {
        alarm_manager.beep_timer = HAL_GetTick();
        alarm_manager.beep_state = false;
        
        printf("[Alarm] Beep mode changed: %d -> %d\r\n", old_mode, alarm_manager.beep_mode);
    }
}

/**
 * @brief  更新GPIO状�?
 */
static void alarm_update_gpio(void)
{
    // ALARM引脚：有任何异常时输出低电平
    bool new_alarm_state = ALARM_HAS_ERROR(alarm_manager.error_flags);
    if (new_alarm_state != alarm_manager.alarm_active)
    {
        alarm_manager.alarm_active = new_alarm_state;
        HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, 
                         new_alarm_state ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }
    
    // BEEP引脚：根据蜂鸣器状态控�?
    GPIO_PinState beep_pin_state = alarm_manager.beep_state ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, beep_pin_state);
}

/**
 * @brief  蜂鸣器脉冲处�?
 * @note   根据当前模式生成非阻塞脉�?
 */
static void alarm_beep_pulse_handler(void)
{
    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed = current_time - alarm_manager.beep_timer;
    
    switch (alarm_manager.beep_mode)
    {
        case BEEP_MODE_OFF:
            // 关闭模式 - 静音
            alarm_manager.beep_state = false;
            break;
            
        case BEEP_MODE_CONTINUOUS:
            // 持续模式 - 一直响
            alarm_manager.beep_state = true;
            break;
            
        case BEEP_MODE_PULSE_50MS:
            // 50ms脉冲模式�?0ms�?+ 50ms�?
            if (elapsed < BEEP_PULSE_ON_50MS)
            {
                alarm_manager.beep_state = true;  // �?
            }
            else if (elapsed < BEEP_PULSE_PERIOD_50MS)
            {
                alarm_manager.beep_state = false; // �?
            }
            else
            {
                // 周期结束，重置定时器
                alarm_manager.beep_timer = current_time;
                alarm_manager.beep_state = true;
            }
            break;
            
        case BEEP_MODE_PULSE_1S:
            // 1秒脉冲模式：1s�?+ 1s�?
            if (elapsed < BEEP_PULSE_ON_1S)
            {
                alarm_manager.beep_state = true;  // �?
            }
            else if (elapsed < BEEP_PULSE_PERIOD_1S)
            {
                alarm_manager.beep_state = false; // �?
            }
            else
            {
                // 周期结束，重置定时器
                alarm_manager.beep_timer = current_time;
                alarm_manager.beep_state = true;
            }
            break;
            
        default:
            alarm_manager.beep_state = false;
            break;
    }
}

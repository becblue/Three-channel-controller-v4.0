/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature.h
  * @brief          : 温度检测与风扇控制模块头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本模块实现3路NTC温度采集、滤波处理、温度阈值判断和风扇PWM控制
  *
  * 功能特性：
  * - ADC+DMA连续采集3路NTC（PA0、PA1、PA2）
  * - 移动平均滤波（8点平均）
  * - 温度阈值：35°C（低）、60°C（高）
  * - 温度回差：2°C
  * - 风扇PWM：50%（正常）、95%（高温）
  * - 更新周期：1秒
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 温度管理器数据结构
 */
typedef struct {
    uint16_t adc_raw[3];        ///< 原始ADC值（未滤波）
    uint16_t adc_filtered[3];   ///< 滤波后的ADC值
    int16_t temperature[3];     ///< 温度值（0.1°C单位）
    uint8_t fan_speed;          ///< 风扇PWM占空比（0-100%）
    uint16_t fan_rpm;           ///< 风扇实际转速（RPM）
    uint8_t overheat_flag[3];   ///< 过温标志（0-正常，1-35°C告警，2-60°C告警）
    bool initialized;           ///< 初始化标志
} Temperature_Manager_t;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief 滤波参数
 */
#define TEMP_FILTER_SIZE        8       ///< 移动平均滤波窗口大小

/**
 * @brief NTC通道数量
 */
#define TEMP_CHANNEL_COUNT      3       ///< 3路NTC

/**
 * @brief 风扇PWM占空比定义
 */
#define FAN_SPEED_NORMAL        50      ///< 正常风扇速度：50%
#define FAN_SPEED_HIGH          95      ///< 高速风扇速度：95%

/**
 * @brief 风扇转速测量参数
 */
#define FAN_PULSE_PER_REV       2       ///< 每转脉冲数（根据风扇规格确定）

/**
 * @brief 过温标志定义
 */
#define TEMP_STATUS_NORMAL      0       ///< 正常（<35°C）
#define TEMP_STATUS_WARM        1       ///< 温暖（35°C~60°C）
#define TEMP_STATUS_OVERHEAT    2       ///< 过热（>60°C）

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief 温度判断宏
 */
#define IS_TEMP_NORMAL(t)       ((t) < TEMP_THRESHOLD_LOW)
#define IS_TEMP_WARM(t)         ((t) >= TEMP_THRESHOLD_LOW && (t) < TEMP_THRESHOLD_HIGH)
#define IS_TEMP_OVERHEAT(t)     ((t) >= TEMP_THRESHOLD_HIGH)

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  温度模块初始化
 * @note   启动ADC+DMA连续采集，初始化滤波器，启动风扇PWM
 * @retval None
 */
void Temperature_Init(void);

/**
 * @brief  温度数据更新
 * @note   从DMA缓冲区读取ADC值，进行滤波、温度转换、阈值判断
 *         建议每1秒调用一次
 * @retval None
 */
void Temperature_Update(void);

/**
 * @brief  获取温度值
 * @param  t1: 输出NTC1温度（0.1°C单位）
 * @param  t2: 输出NTC2温度（0.1°C单位）
 * @param  t3: 输出NTC3温度（0.1°C单位）
 * @retval None
 */
void Temperature_GetValues(int16_t *t1, int16_t *t2, int16_t *t3);

/**
 * @brief  获取风扇速度
 * @retval 风扇PWM占空比（0-100%）
 */
uint8_t Temperature_GetFanSpeed(void);

/**
 * @brief  获取过温标志
 * @param  channel: 通道号（0-2）
 * @retval 过温标志（0-正常，1-35°C告警，2-60°C告警）
 */
uint8_t Temperature_GetOverheatFlag(uint8_t channel);

/**
 * @brief  打印温度状态（调试用）
 * @note   输出当前3路温度、ADC值、风扇速度等信息
 * @retval None
 */
void Temperature_PrintStatus(void);

/**
 * @brief  设置风扇速度（手动控制，调试用）
 * @param  speed: 风扇PWM占空比（0-100%）
 * @retval None
 */
void Temperature_SetFanSpeed(uint8_t speed);

/**
 * @brief  获取温度管理器指针（调试用）
 * @retval 温度管理器数据结构指针
 */
const Temperature_Manager_t* Temperature_GetManager(void);

/**
 * @brief  获取风扇转速（RPM）
 * @retval 风扇转速（转/分钟）
 */
uint16_t Temperature_GetFanRPM(void);

/**
 * @brief  风扇脉冲中断处理（由外部中断调用）
 * @note   每检测到FAN_SEN下降沿时调用
 * @retval None
 */
void Temperature_FanPulseISR(void);

/**
 * @brief  1秒定时更新（由定时器或主循环调用）
 * @note   计算风扇转速，清零脉冲计数器
 * @retval None
 */
void Temperature_1sHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEMPERATURE_H */

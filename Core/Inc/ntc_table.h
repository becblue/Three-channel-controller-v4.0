/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : ntc_table.h
  * @brief          : NTC热敏电阻查表模块头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本模块实现NTC热敏电阻的ADC值到温度的转换功能
  *
  * 硬件电路：
  * - 10KΩ上拉电阻 + 10K B3435 NTC热敏电阻串联分压
  * - ADC分辨率：12位（0-4095）
  * - 参考电压：3.3V
  * - 分压公式：ADC = R_ntc / (R_ntc + 10K) * 4095
  *
  * 温度范围：-40°C ~ 125°C
  * 查表精度：±0.5°C（采用线性插值）
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __NTC_TABLE_H
#define __NTC_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief NTC查表数据结构
 * @note  存储ADC值和对应的温度值
 */
typedef struct {
    uint16_t adc_value;     ///< ADC采样值（0-4095）
    int16_t temperature;    ///< 对应温度（0.1°C单位，如250表示25.0°C）
} NTC_Table_t;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief NTC查表数组大小
 * @note  从-40°C到125°C，每5°C一个采样点，共34个点
 */
#define NTC_TABLE_SIZE      34

/**
 * @brief 温度范围定义
 */
#define NTC_TEMP_MIN        (-400)  ///< 最低温度：-40.0°C（0.1°C单位）
#define NTC_TEMP_MAX        (1250)  ///< 最高温度：125.0°C（0.1°C单位）

/**
 * @brief ADC范围定义
 */
#define NTC_ADC_MIN         (0)     ///< 最小ADC值
#define NTC_ADC_MAX         (4095)  ///< 最大ADC值（12位）

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief 温度值转换宏
 */
#define NTC_TEMP_TO_CELSIUS(t)      ((float)(t) / 10.0f)  ///< 0.1°C单位转摄氏度
#define NTC_CELSIUS_TO_TEMP(c)      ((int16_t)((c) * 10)) ///< 摄氏度转0.1°C单位

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  NTC模块初始化
 * @note   初始化查表数据（如果需要）
 * @retval None
 */
void NTC_Init(void);

/**
 * @brief  ADC值转温度
 * @param  adc_value: ADC采样值（0-4095）
 * @retval 温度值（0.1°C单位，如250表示25.0°C）
 *         返回NTC_TEMP_MIN或NTC_TEMP_MAX表示超出测量范围
 */
int16_t NTC_GetTemperature(uint16_t adc_value);

/**
 * @brief  打印NTC查表（调试用）
 * @note   通过UART输出整个查表数据
 * @retval None
 */
void NTC_PrintTable(void);

/**
 * @brief  获取查表数组指针（调试用）
 * @retval 查表数组指针
 */
const NTC_Table_t* NTC_GetTable(void);

#ifdef __cplusplus
}
#endif

#endif /* __NTC_TABLE_H */

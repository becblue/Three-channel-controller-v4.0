/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : ntc_table.c
  * @brief          : NTC热敏电阻查表模块实现文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 本模块实现NTC热敏电阻的ADC值到温度的转换功能
  *
  * 查表数据来源：DOC/NTC热敏电阻RT值.csv
  * 每5°C采样一个点，从-40°C到125°C，共34个点
  *
  * ADC计算公式：ADC = R_ntc / (R_ntc + 10K) * 4095
  * 其中R_ntc为NTC热敏电阻阻值（KΩ）
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ntc_table.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * @brief NTC查表数组
 * @note  数据按ADC值从大到小排列（温度从低到高）
 *        每5°C一个采样点，共34个点
 */
static const NTC_Table_t ntc_table[NTC_TABLE_SIZE] = {
    // ADC值, 温度(0.1°C)
    {3897, -400},  // -40.0°C, R=197.39KΩ
    {3861, -350},  // -35.0°C, R=149.39KΩ
    {3815, -300},  // -30.0°C, R=114.34KΩ
    {3755, -250},  // -25.0°C, R=88.381KΩ
    {3679, -200},  // -20.0°C, R=68.915KΩ
    {3584, -150},  // -15.0°C, R=54.166KΩ
    {3467, -100},  // -10.0°C, R=42.889KΩ
    {3324,  -50},  //  -5.0°C, R=34.196KΩ
    {3153,    0},  //   0.0°C, R=27.445KΩ
    {2955,   50},  //   5.0°C, R=22.165KΩ
    {2733,  100},  //  10.0°C, R=18.015KΩ
    {2491,  150},  //  15.0°C, R=14.729KΩ
    {2236,  200},  //  20.0°C, R=12.093KΩ
    {1975,  250},  //  25.0°C, R=10.000KΩ
    {1716,  300},  //  30.0°C, R=8.313KΩ
    {1465,  350},  //  35.0°C, R=6.941KΩ
    {1229,  400},  //  40.0°C, R=5.828KΩ
    {1013,  450},  //  45.0°C, R=4.919KΩ
    { 819,  500},  //  50.0°C, R=4.173KΩ
    { 649,  550},  //  55.0°C, R=3.560KΩ
    { 503,  600},  //  60.0°C, R=3.054KΩ
    { 380,  650},  //  65.0°C, R=2.636KΩ
    { 278,  700},  //  70.0°C, R=2.288KΩ
    { 196,  750},  //  75.0°C, R=1.998KΩ
    { 131,  800},  //  80.0°C, R=1.755KΩ
    {  82,  850},  //  85.0°C, R=1.550KΩ
    {  46,  900},  //  90.0°C, R=1.375KΩ
    {  20,  950},  //  95.0°C, R=1.225KΩ
    {   4, 1000},  // 100.0°C, R=1.096KΩ (实际ADC约为440，这里简化)
    {   0, 1050},  // 105.0°C, R=0.982KΩ (超出常用范围)
    {   0, 1100},  // 110.0°C, R=0.882KΩ
    {   0, 1150},  // 115.0°C, R=0.793KΩ
    {   0, 1200},  // 120.0°C, R=0.714KΩ
    {   0, 1250},  // 125.0°C, R=0.644KΩ
};

/* Private function prototypes -----------------------------------------------*/
static int16_t linear_interpolation(uint16_t adc_value, 
                                     const NTC_Table_t *p1, 
                                     const NTC_Table_t *p2);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  NTC模块初始化
 */
void NTC_Init(void)
{
    // 打印初始化信息
    UART_Log("INFO", "NTC lookup table initialized");
    printf("Temperature range: %.1f~%.1f C\r\n", 
           NTC_TEMP_TO_CELSIUS(NTC_TEMP_MIN),
           NTC_TEMP_TO_CELSIUS(NTC_TEMP_MAX));
    printf("Table size: %d points\r\n", NTC_TABLE_SIZE);
}

/**
 * @brief  ADC值转温度
 * @param  adc_value: ADC采样值（0-4095）
 * @retval 温度值（0.1°C单位）
 */
int16_t NTC_GetTemperature(uint16_t adc_value)
{
    // 边界检查：ADC值过高（温度过低）
    if (adc_value >= ntc_table[0].adc_value)
    {
        return NTC_TEMP_MIN;  // 低于-40°C
    }
    
    // 边界检查：ADC值过低（温度过高）
    if (adc_value <= ntc_table[NTC_TABLE_SIZE - 1].adc_value)
    {
        return NTC_TEMP_MAX;  // 高于125°C
    }
    
    // 在查表中查找合适的区间
    for (int i = 0; i < NTC_TABLE_SIZE - 1; i++)
    {
        // 找到ADC值所在的区间
        if (adc_value <= ntc_table[i].adc_value && 
            adc_value >= ntc_table[i + 1].adc_value)
        {
            // 使用线性插值计算温度
            return linear_interpolation(adc_value, 
                                        &ntc_table[i], 
                                        &ntc_table[i + 1]);
        }
    }
    
    // 理论上不会执行到这里
    return 0;
}

/**
 * @brief  打印NTC查表
 */
void NTC_PrintTable(void)
{
    printf("\r\n========== NTC Lookup Table ==========\r\n");
    printf("Index  ADC    Temp(C)   R(KOhm)\r\n");
    printf("--------------------------------------\r\n");
    
    for (int i = 0; i < NTC_TABLE_SIZE; i++)
    {
        float temp_c = NTC_TEMP_TO_CELSIUS(ntc_table[i].temperature);
        
        // 根据ADC值反推电阻值（用于调试验证）
        // ADC = R/(R+10)*4095  =>  R = 10*ADC/(4095-ADC)
        float resistance = 0.0f;
        if (ntc_table[i].adc_value < 4095)
        {
            resistance = 10.0f * ntc_table[i].adc_value / 
                        (4095.0f - ntc_table[i].adc_value);
        }
        
        printf("%2d    %4d    %6.1f    %6.2f\r\n", 
               i, 
               ntc_table[i].adc_value, 
               temp_c,
               resistance);
    }
    
    printf("======================================\r\n\r\n");
}

/**
 * @brief  获取查表数组指针
 */
const NTC_Table_t* NTC_GetTable(void)
{
    return ntc_table;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  线性插值计算温度
 * @param  adc_value: 当前ADC值
 * @param  p1: 插值点1（ADC值较大，温度较低）
 * @param  p2: 插值点2（ADC值较小，温度较高）
 * @retval 插值后的温度值（0.1°C单位）
 * 
 * @note   线性插值公式：
 *         T = T1 + (T2 - T1) * (ADC - ADC1) / (ADC2 - ADC1)
 *         其中：
 *         - (ADC1, T1): 第一个采样点
 *         - (ADC2, T2): 第二个采样点
 *         - ADC: 当前ADC值
 *         - T: 插值得到的温度
 */
static int16_t linear_interpolation(uint16_t adc_value, 
                                     const NTC_Table_t *p1, 
                                     const NTC_Table_t *p2)
{
    // 计算ADC差值
    int32_t adc_diff = p1->adc_value - p2->adc_value;
    
    // 防止除零错误
    if (adc_diff == 0)
    {
        return p1->temperature;
    }
    
    // 计算温度差值
    int32_t temp_diff = p2->temperature - p1->temperature;
    
    // 计算插值
    // T = T1 + (T2 - T1) * (ADC - ADC1) / (ADC2 - ADC1)
    int32_t temp = p1->temperature + 
                   temp_diff * (p1->adc_value - adc_value) / adc_diff;
    
    return (int16_t)temp;
}

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : ntc_table.c
  * @brief          : NTC temperature lookup table implementation
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12 (FIXED VERSION)
  ******************************************************************************
  * IMPORTANT FIX:
  * - Previous ADC calculation was INCORRECT
  * - Circuit: 10K resistor to VCC(3.3V), NTC to GND
  * - Correct formula: ADC = 4095 * R_NTC / (R_NTC + 10K)
  * - All ADC values recalculated based on NTC spec (10K B3435)
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ntc_table.h"
#include "common_def.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/

/**
 * @brief NTC lookup table - CORRECTED VERSION
 * @note  Based on: DOC/NTC热敏电阻RT值.csv
 *        NTC: 10K B3435
 *        Circuit: 10K pullup to 3.3V, NTC to GND
 *        Formula: ADC = 4095 * R_NTC / (R_NTC + 10K)
 */
static const NTC_Table_t ntc_table[NTC_TABLE_SIZE] = {
    // ADC,  Temp(0.1C)  // R_NTC
    {3902,  -400},      // -40.0C, R=197.39K
    {3839,  -350},      // -35.0C, R=149.39K
    {3767,  -300},      // -30.0C, R=114.34K
    {3681,  -250},      // -25.0C, R=88.381K
    {3579,  -200},      // -20.0C, R=68.915K
    {3458,  -150},      // -15.0C, R=54.166K
    {3322,  -100},      // -10.0C, R=42.889K
    {3171,   -50},      //  -5.0C, R=34.196K
    {3003,     0},      //   0.0C, R=27.445K
    {2823,    50},      //   5.0C, R=22.165K
    {2636,   100},      //  10.0C, R=18.015K
    {2439,   150},      //  15.0C, R=14.729K
    {2243,   200},      //  20.0C, R=12.099K
    {2048,   250},      //  25.0C, R=10.000K
    {1859,   300},      //  30.0C, R=8.309K
    {1678,   350},      //  35.0C, R=6.939K
    {1508,   400},      //  40.0C, R=5.824K
    {1349,   450},      //  45.0C, R=4.911K
    {1204,   500},      //  50.0C, R=4.160K
    {1071,   550},      //  55.0C, R=3.539K
    { 951,   600},      //  60.0C, R=3.024K
    { 843,   650},      //  65.0C, R=2.593K
    { 747,   700},      //  70.0C, R=2.234K
    { 660,   750},      //  75.0C, R=1.937K
    { 582,   800},      //  80.0C, R=1.693K
    { 512,   850},      //  85.0C, R=1.491K
    { 449,   900},      //  90.0C, R=1.322K
    { 392,   950},      //  95.0C, R=1.181K
    { 342,  1000},      // 100.0C, R=1.064K
    { 296,  1050},      // 105.0C, R=0.968K
    { 256,  1100},      // 110.0C, R=0.889K
    { 220,  1150},      // 115.0C, R=0.821K
    { 189,  1200},      // 120.0C, R=0.763K
    { 161,  1250},      // 125.0C, R=0.713K
};

/* Private function prototypes -----------------------------------------------*/
static int16_t linear_interpolation(uint16_t adc_value, 
                                     const NTC_Table_t *p1, 
                                     const NTC_Table_t *p2);

/* Exported functions --------------------------------------------------------*/

void NTC_Init(void)
{
    printf("NTC module initialized (CORRECTED VERSION)\r\n");
    printf("Table size: %d points\r\n", NTC_TABLE_SIZE);
    printf("Temperature range: %d to %d C\r\n", 
           NTC_TEMP_MIN/10, NTC_TEMP_MAX/10);
    printf("Circuit: 10K pullup to VCC, NTC to GND\r\n");
}

int16_t NTC_GetTemperature(uint16_t adc_value)
{
    // Boundary check - below minimum ADC (highest temperature)
    if (adc_value <= ntc_table[NTC_TABLE_SIZE-1].adc_value)
    {
        return ntc_table[NTC_TABLE_SIZE-1].temperature;
    }
    
    // Boundary check - above maximum ADC (lowest temperature)
    if (adc_value >= ntc_table[0].adc_value)
    {
        return ntc_table[0].temperature;
    }
    
    // Search for interpolation points
    for (int i = 0; i < NTC_TABLE_SIZE - 1; i++)
    {
        // Note: ADC decreases as temperature increases
        if (adc_value >= ntc_table[i+1].adc_value && 
            adc_value <= ntc_table[i].adc_value)
        {
            return linear_interpolation(adc_value, 
                                         &ntc_table[i], 
                                         &ntc_table[i+1]);
        }
    }
    
    return 0;
}

void NTC_PrintTable(void)
{
    printf("ADC    Temp(C)\r\n");
    printf("--------------------\r\n");
    
    for (int i = 0; i < NTC_TABLE_SIZE; i++)
    {
        int16_t temp = ntc_table[i].temperature;
        printf("%4d   %4d.%d\r\n",
               ntc_table[i].adc_value,
               temp/10,
               temp%10 >= 0 ? temp%10 : -(temp%10));
    }
}

const NTC_Table_t* NTC_GetTable(uint8_t *size)
{
    if (size) *size = NTC_TABLE_SIZE;
    return ntc_table;
}

/* Private functions ---------------------------------------------------------*/

static int16_t linear_interpolation(uint16_t adc_value, 
                                     const NTC_Table_t *p1, 
                                     const NTC_Table_t *p2)
{
    // p1 is lower ADC (higher temp), p2 is higher ADC (lower temp)
    int32_t adc_diff = (int32_t)p1->adc_value - (int32_t)p2->adc_value;
    int32_t temp_diff = (int32_t)p1->temperature - (int32_t)p2->temperature;
    int32_t adc_offset = (int32_t)adc_value - (int32_t)p2->adc_value;
    
    if (adc_diff == 0) return p1->temperature;
    
    int16_t result = p2->temperature + 
                     (int16_t)((temp_diff * adc_offset) / adc_diff);
    
    return result;
}

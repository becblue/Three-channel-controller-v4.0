/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature.c
  * @brief          : Temperature monitoring and fan control module
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "temperature.h"
#include "common_def.h"
#include "ntc_table.h"
#include "usart.h"
#include "adc.h"
#include "tim.h"
#include <string.h>
#include <stdlib.h>

/* Private variables ---------------------------------------------------------*/

static Temperature_Manager_t temp_manager = {0};
static uint16_t adc_filter_buffer[TEMP_CHANNEL_COUNT][TEMP_FILTER_SIZE] = {0};
static uint8_t filter_index = 0;
static bool filter_ready = false;

extern uint16_t adc_dma_buffer[3];

/* Private function prototypes -----------------------------------------------*/
static void update_filter(uint8_t channel, uint16_t adc_value);
static uint16_t get_filtered_value(uint8_t channel);
static void update_fan_control(void);
static void update_overheat_flags(void);

/* Exported functions --------------------------------------------------------*/

void Temperature_Init(void)
{
    memset(&temp_manager, 0, sizeof(Temperature_Manager_t));
    memset(adc_filter_buffer, 0, sizeof(adc_filter_buffer));
    filter_index = 0;
    filter_ready = false;
    
    // ADC calibration (required for STM32F1)
    HAL_ADCEx_Calibration_Start(&hadc1);
    
    // Start ADC+DMA
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, TEMP_CHANNEL_COUNT) != HAL_OK)
    {
        printf("ERROR: ADC+DMA start failed\r\n");
        return;
    }
    
    // Start PWM
    if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK)
    {
        printf("ERROR: TIM3 PWM start failed\r\n");
        return;
    }
    
    temp_manager.fan_speed = FAN_SPEED_NORMAL;
    Temperature_SetFanSpeed(FAN_SPEED_NORMAL);
    temp_manager.initialized = true;
    
    UART_Log("INFO", "Temperature module initialized");
    printf("ADC+DMA started for 3 NTC channels\r\n");
    printf("Fan PWM started at %d%%\r\n", temp_manager.fan_speed);
    printf("Filter size: %d points\r\n", TEMP_FILTER_SIZE);
}

void Temperature_Update(void)
{
    if (!temp_manager.initialized) return;
    
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        temp_manager.adc_raw[i] = adc_dma_buffer[i];
        update_filter(i, temp_manager.adc_raw[i]);
    }
    
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        temp_manager.adc_filtered[i] = get_filtered_value(i);
    }
    
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        temp_manager.temperature[i] = NTC_GetTemperature(temp_manager.adc_filtered[i]);
    }
    
    update_overheat_flags();
    update_fan_control();
}

void Temperature_GetValues(int16_t *t1, int16_t *t2, int16_t *t3)
{
    if (t1) *t1 = temp_manager.temperature[0];
    if (t2) *t2 = temp_manager.temperature[1];
    if (t3) *t3 = temp_manager.temperature[2];
}

uint8_t Temperature_GetFanSpeed(void)
{
    return temp_manager.fan_speed;
}

uint8_t Temperature_GetOverheatFlag(uint8_t channel)
{
    if (channel < TEMP_CHANNEL_COUNT)
        return temp_manager.overheat_flag[channel];
    return 0;
}

void Temperature_PrintStatus(void)
{
    printf("\r\n========== Temperature Status ==========\r\n");
    printf("NTC Temperatures:\r\n");
    
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        int16_t temp = temp_manager.temperature[i];
        printf("  NTC%d: %3d.%d C  (ADC: %4d", 
               i+1, temp/10, abs(temp%10), temp_manager.adc_filtered[i]);
        
        if (temp_manager.overheat_flag[i] == TEMP_STATUS_OVERHEAT)
            printf(" - OVERHEAT!)");
        else if (temp_manager.overheat_flag[i] == TEMP_STATUS_WARM)
            printf(" - Warm");
        else
            printf(" - Normal");
        printf("\r\n");
    }
    
    printf("\r\nFan Control:\r\n");
    printf("  Speed: %d%%", temp_manager.fan_speed);
    if (temp_manager.fan_speed >= FAN_SPEED_HIGH)
        printf(" (HIGH SPEED)");
    else
        printf(" (NORMAL)");
    printf("\r\n");
    
    printf("\r\nFilter Status:\r\n");
    printf("  Filter ready: %s\r\n", filter_ready ? "YES" : "NO");
    printf("  Filter index: %d/%d\r\n", filter_index, TEMP_FILTER_SIZE);
    printf("========================================\r\n\r\n");
}

void Temperature_SetFanSpeed(uint8_t speed)
{
    if (speed > 100) speed = 100;
    temp_manager.fan_speed = speed;
    
    // TIM3 ARR=99, so CCR = ARR * duty / 100
    uint16_t ccr = (uint16_t)(99 * speed / 100);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr);
}

const Temperature_Manager_t* Temperature_GetManager(void)
{
    return &temp_manager;
}

/* Private functions ---------------------------------------------------------*/

static void update_filter(uint8_t channel, uint16_t adc_value)
{
    if (channel >= TEMP_CHANNEL_COUNT) return;
    
    adc_filter_buffer[channel][filter_index] = adc_value;
    
    if (channel == TEMP_CHANNEL_COUNT - 1)
    {
        filter_index++;
        if (filter_index >= TEMP_FILTER_SIZE)
        {
            filter_index = 0;
            filter_ready = true;
        }
    }
}

static uint16_t get_filtered_value(uint8_t channel)
{
    if (channel >= TEMP_CHANNEL_COUNT) return 0;
    
    if (!filter_ready)
    {
        return adc_filter_buffer[channel][filter_index > 0 ? filter_index - 1 : 0];
    }
    
    uint32_t sum = 0;
    for (int i = 0; i < TEMP_FILTER_SIZE; i++)
    {
        sum += adc_filter_buffer[channel][i];
    }
    
    return (uint16_t)(sum / TEMP_FILTER_SIZE);
}

static void update_overheat_flags(void)
{
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        int16_t temp = temp_manager.temperature[i];
        uint8_t current_status = temp_manager.overheat_flag[i];
        
        switch (current_status)
        {
            case TEMP_STATUS_NORMAL:
                if (temp >= TEMP_THRESHOLD_LOW)
                {
                    temp_manager.overheat_flag[i] = TEMP_STATUS_WARM;
                    printf("NTC%d: Temp reached 35C\r\n", i+1);
                }
                break;
                
            case TEMP_STATUS_WARM:
                if (temp < (TEMP_THRESHOLD_LOW - TEMP_HYSTERESIS))
                {
                    temp_manager.overheat_flag[i] = TEMP_STATUS_NORMAL;
                    printf("NTC%d: Temp back to normal\r\n", i+1);
                }
                else if (temp >= TEMP_THRESHOLD_HIGH)
                {
                    temp_manager.overheat_flag[i] = TEMP_STATUS_OVERHEAT;
                    printf("NTC%d: OVERHEAT! 60C reached\r\n", i+1);
                }
                break;
                
            case TEMP_STATUS_OVERHEAT:
                if (temp < (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS))
                {
                    temp_manager.overheat_flag[i] = TEMP_STATUS_WARM;
                    printf("NTC%d: Temp decreased below 60C\r\n", i+1);
                }
                break;
                
            default:
                temp_manager.overheat_flag[i] = TEMP_STATUS_NORMAL;
                break;
        }
    }
}

static void update_fan_control(void)
{
    int16_t max_temp = temp_manager.temperature[0];
    for (int i = 1; i < TEMP_CHANNEL_COUNT; i++)
    {
        if (temp_manager.temperature[i] > max_temp)
            max_temp = temp_manager.temperature[i];
    }
    
    bool has_overheat = false;
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        if (temp_manager.overheat_flag[i] == TEMP_STATUS_OVERHEAT)
        {
            has_overheat = true;
            break;
        }
    }
    
    uint8_t new_speed = temp_manager.fan_speed;
    
    if (has_overheat)
    {
        new_speed = FAN_SPEED_HIGH;
    }
    else
    {
        if (temp_manager.fan_speed == FAN_SPEED_HIGH)
        {
            if (max_temp < (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS))
                new_speed = FAN_SPEED_NORMAL;
        }
        else
        {
            new_speed = FAN_SPEED_NORMAL;
        }
    }
    
    if (new_speed != temp_manager.fan_speed)
    {
        printf("Fan speed: %d%% -> %d%%\r\n", temp_manager.fan_speed, new_speed);
        Temperature_SetFanSpeed(new_speed);
    }
}

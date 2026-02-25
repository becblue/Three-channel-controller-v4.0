/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature.c
  * @brief          : Temperature monitoring and fan control (REFACTORED)
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  * @note           : Refactored based on reference code
  *                   Calculation flow: ADC -> Voltage -> Resistance -> Temperature
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "temperature.h"
#include "common_def.h"
#include "relay_control.h"
#include "usart.h"
#include "adc.h"
#include "tim.h"
#include <string.h>
#include <stdlib.h>

/* Private define ------------------------------------------------------------*/
#define NTC_PULLUP_RES  10.0f   // Pull-up resistor 10kOhm
#define NTC_VREF        3.3f    // Reference voltage 3.3V

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    float temp;      // Temperature (degC)
    float r_ntc;     // NTC resistance (kOhm)
} NTC_Table_Entry_t;

/* Private variables ---------------------------------------------------------*/

// Temperature manager
static Temperature_Manager_t temp_manager = {0};

// 妞嬪孩澧栨潪顒勨偓鐔哥ゴ闁诧拷
volatile uint32_t fan_pulse_count    = 0U;  /* 1s 閸愬懏婀侀弫鍫ｅ墻閸愯尪顓搁弫甯礄ISR 缁鳖垰濮為敍锟� */
static   uint32_t last_update_tick   = 0U;  /* 娑撳﹥顐� 1s 缂佺喕顓搁弮璺哄煝 */

// ISR 闂冨弶濮堥敍姘愁唶瑜版洑绗傚▎鈩冩箒閺佸牐鍓﹂崘鑼畱閺冭泛鍩㈤敍宀冪箖濠婏拷 < FAN_DEBOUNCE_MIN_MS 閻ㄥ嫬娅旀竟锟�
static volatile uint32_t s_last_pulse_tick = 0U;

// RPM 濠婃垵濮╅獮鍐叉綆缂傛挸鍟块崠鐚寸礄瀵邦亞骞嗛梼鐔峰灙閿涘瞼鐛ラ崣锝呫亣鐏忥拷 FAN_RPM_AVG_SIZE閿涳拷
static uint16_t s_rpm_buf[FAN_RPM_AVG_SIZE] = {0U};
static uint8_t  s_rpm_buf_idx  = 0U;   /* 娑撳顐奸崘娆忓弳娴ｅ秶鐤� */
static uint8_t  s_rpm_buf_cnt  = 0U;   /* 瀹告彃锝為崗銉ф畱閺堝鏅ラ弽閿嬫拱閺佸府绱欐稉濠囨 FAN_RPM_AVG_SIZE閿涳拷 */

// NTC lookup table: Temperature -> Resistance (based on CSV data)
static const NTC_Table_Entry_t ntc_table[] = {
    {-40,197.39},{-39,186.54},{-38,176.35},{-37,166.8},{-36,157.82},{-35,149.39},{-34,141.51},{-33,134.09},{-32,127.11},{-31,120.53},
    {-30,114.34},{-29,108.53},{-28,103.04},{-27,97.87},{-26,92.989},{-25,88.381},{-24,84.036},{-23,79.931},{-22,76.052},{-21,72.384},
    {-20,68.915},{-19,65.634},{-18,62.529},{-17,59.589},{-16,56.804},{-15,54.166},{-14,51.665},{-13,49.294},{-12,47.046},{-11,44.913},
    {-10,42.889},{-9,40.967},{-8,39.142},{-7,37.408},{-6,35.761},{-5,34.196},{-4,32.707},{-3,31.291},{-2,29.945},{-1,28.664},
    {0,27.445},{1,26.283},{2,25.177},{3,24.124},{4,23.121},{5,22.165},{6,21.253},{7,20.384},{8,19.555},{9,18.764},
    {10,18.01},{11,17.29},{12,16.602},{13,15.946},{14,15.319},{15,14.72},{16,14.148},{17,13.601},{18,13.078},{19,12.578},
    {20,12.099},{21,11.642},{22,11.204},{23,10.785},{24,10.384},{25,10},{26,9.632},{27,9.28},{28,8.943},{29,8.619},
    {30,8.309},{31,8.012},{32,7.727},{33,7.453},{34,7.191},{35,6.939},{36,6.698},{37,6.466},{38,6.243},{39,6.029},
    {40,5.824},{41,5.627},{42,5.437},{43,5.255},{44,5.08},{45,4.911},{46,4.749},{47,4.593},{48,4.443},{49,4.299},
    {50,4.16},{51,4.027},{52,3.898},{53,3.774},{54,3.654},{55,3.539},{56,3.429},{57,3.322},{58,3.219},{59,3.119},
    {60,3.024},{61,2.931},{62,2.842},{63,2.756},{64,2.673},{65,2.593},{66,2.516},{67,2.441},{68,2.369},{69,2.3},
    {70,2.233},{71,2.168},{72,2.105},{73,2.044},{74,1.986},{75,1.929},{76,1.874},{77,1.821},{78,1.77},{79,1.72},
    {80,1.673},{81,1.626},{82,1.581},{83,1.538},{84,1.496},{85,1.455},{86,1.416},{87,1.377},{88,1.34},{89,1.304},
    {90,1.27},{91,1.236},{92,1.204},{93,1.172},{94,1.141},{95,1.112},{96,1.083},{97,1.055},{98,1.028},{99,1.002},
    {100,0.976},{101,0.951},{102,0.927},{103,0.904},{104,0.882},{105,0.86},{106,0.838},{107,0.818},{108,0.798},{109,0.778},
    {110,0.759},{111,0.741},{112,0.723},{113,0.706},{114,0.689},{115,0.673},{116,0.657},{117,0.641},{118,0.626},{119,0.612},
    {120,0.598},{121,0.584},{122,0.57},{123,0.557},{124,0.545},{125,0.532}
};
#define NTC_TABLE_SIZE (sizeof(ntc_table)/sizeof(ntc_table[0]))

// DMA buffer (defined in adc.c)
extern uint16_t adc_dma_buffer[3];

/* Private function prototypes -----------------------------------------------*/
static float calc_ntc_resistance(float voltage);
static float ntc_res_to_temp(float r_ntc);
static void update_overheat_flags(void);
static void update_fan_control(void);

/* Exported functions --------------------------------------------------------*/

void Temperature_Init(void)
{
    memset(&temp_manager, 0, sizeof(Temperature_Manager_t));
    
    // ADC calibration
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
    
    // Initialize fan speed measurement
    temp_manager.fan_rpm = 0;
    fan_pulse_count = 0;
    last_update_tick = HAL_GetTick();
    
    printf("[Temperature] Module initialized\r\n");
    printf("[Temperature] ADC+DMA started for %d channels\r\n", TEMP_CHANNEL_COUNT);
    printf("[Temperature] Fan PWM started at %d%%\r\n", temp_manager.fan_speed);
    printf("[Temperature] Lookup table: %d points\r\n", NTC_TABLE_SIZE);
    printf("[Temperature] Calculation: ADC->Voltage->Resistance->Temperature\r\n");
    printf("[Temperature] Fan RPM measurement enabled (PC12)\r\n");
}

void Temperature_Update(void)
{
    if (!temp_manager.initialized) return;
    
    // Check if 1 second has elapsed for RPM calculation
    uint32_t current_tick = HAL_GetTick();
    if ((current_tick - last_update_tick) >= 1000) {
        Temperature_1sHandler();
        last_update_tick = current_tick;
    }
    
    // Process all channels: ADC -> Voltage -> Resistance -> Temperature
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        // Step 1: Read ADC raw value
        temp_manager.adc_raw[i] = adc_dma_buffer[i];
        
        // Step 2: Convert ADC to voltage
        float voltage = (float)temp_manager.adc_raw[i] * NTC_VREF / 4095.0f;
        
        // Step 3: Calculate NTC resistance
        float r_ntc = calc_ntc_resistance(voltage);
        
        // Step 4: Lookup temperature from resistance
        float temp = ntc_res_to_temp(r_ntc);
        
        // Step 5: Store temperature (convert to 0.1C unit)
        temp_manager.temperature[i] = (int16_t)(temp * 10.0f);
        
        // Store filtered ADC (no filtering for now, direct copy)
        temp_manager.adc_filtered[i] = temp_manager.adc_raw[i];
    }
    
    // Update overheat flags and fan control
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

/**
 * @brief Get fan RPM
 */
uint16_t Temperature_GetFanRPM(void)
{
    return temp_manager.fan_rpm;
}

/**
 * @brief  椋庢墖鑴夊啿涓柇澶勭悊锛團AN_SEN 涓嬮檷娌胯Е鍙戯級
 * @note   鍔犲叆鏃堕棿闃叉姈锛氫袱娆′腑鏂棿闅� < FAN_DEBOUNCE_MIN_MS 鏃惰涓� EMI 鍣０锛屼涪寮冦€�
 *         缁х數鍣ㄥ垏鎹� EMI < 1ms锛屾甯歌剦鍐叉渶楂橀鐜� ~133Hz锛�7.5ms闂撮殧锛夛紝4ms 鍙噯纭尯鍒嗐€�
 */
void Temperature_FanPulseISR(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - s_last_pulse_tick) >= FAN_DEBOUNCE_MIN_MS)
    {
        fan_pulse_count++;
        s_last_pulse_tick = now;
    }
    /* else: 闂撮殧 < 4ms锛岃涓虹户鐢靛櫒鍒囨崲 EMI 鍣０锛屼涪寮� */
}

/**
 * @brief  1s 风扇转速统计（每秒调用一次）
 *
 * 三项保护逻辑：
 *  1. 继电器忙碌时跳过：500ms 脉冲期间 EMI 可能污染计数，直接丢弃本次样本，
 *     保持上次 RPM 显示值不变，避免 OLED 出现转速跳零。
 *  2. EXTI 防抖已在 ISR 层过滤 < 4ms 噪声脉冲，本函数收到的是干净计数。
 *  3. 滑动平均（窗口 FAN_RPM_AVG_SIZE）：对最近 N 次统计结果取均值，
 *     平滑因单次统计误差引起的显示抖动。
 */
void Temperature_1sHandler(void)
{
    /* ① 继电器切换期间跳过：清零受污染的计数，保持上次 RPM */
    if (Relay_IsChannelBusy(CHANNEL_1) ||
        Relay_IsChannelBusy(CHANNEL_2) ||
        Relay_IsChannelBusy(CHANNEL_3))
    {
        fan_pulse_count = 0U;
        return;
    }

    /* ② 计算本次原始 RPM，清零计数器 */
    uint16_t raw_rpm = (uint16_t)((fan_pulse_count * 60U) / (uint32_t)FAN_PULSE_PER_REV);
    fan_pulse_count  = 0U;

    /* ③ 写入循环缓冲区 */
    s_rpm_buf[s_rpm_buf_idx] = raw_rpm;
    s_rpm_buf_idx = (uint8_t)((s_rpm_buf_idx + 1U) % FAN_RPM_AVG_SIZE);
    if (s_rpm_buf_cnt < (uint8_t)FAN_RPM_AVG_SIZE)
    {
        s_rpm_buf_cnt++;
    }

    /* ④ 计算滑动平均并更新输出值 */
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < s_rpm_buf_cnt; i++)
    {
        sum += s_rpm_buf[i];
    }
    temp_manager.fan_rpm = (uint16_t)(sum / (uint32_t)s_rpm_buf_cnt);
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
    printf("Channel | ADC  | Voltage | R_NTC  | Temp   | Status\r\n");
    printf("--------|------|---------|--------|--------|---------\r\n");
    
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        uint16_t adc = temp_manager.adc_raw[i];
        float voltage = (float)adc * NTC_VREF / 4095.0f;
        float r_ntc = calc_ntc_resistance(voltage);
        int16_t temp = temp_manager.temperature[i];
        
        printf("  NTC%d  | %4d | %5.3fV | %6.2fK | %3d.%dC | ",
               i+1, adc, voltage, r_ntc, temp/10, abs(temp%10));
        
        if (temp_manager.overheat_flag[i] == TEMP_STATUS_OVERHEAT)
            printf("OVERHEAT!");
        else if (temp_manager.overheat_flag[i] == TEMP_STATUS_WARM)
            printf("Warm");
        else
            printf("Normal");
        printf("\r\n");
    }
    
    printf("\r\nFan Control:\r\n");
    printf("  Speed: %d%%", temp_manager.fan_speed);
    if (temp_manager.fan_speed >= FAN_SPEED_HIGH)
        printf(" (HIGH SPEED)");
    else
        printf(" (NORMAL)");
    printf(" | RPM: %d\r\n", temp_manager.fan_rpm);
    printf("========================================\r\n\r\n");
}

void Temperature_SetFanSpeed(uint8_t speed)
{
    if (speed > 100) speed = 100;
    temp_manager.fan_speed = speed;
    
    // Calculate CCR based on ARR
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim3);
    uint32_t ccr = (speed * (arr + 1)) / 100;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr);
}

const Temperature_Manager_t* Temperature_GetManager(void)
{
    return &temp_manager;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Calculate NTC resistance from voltage
 * @param  voltage: ADC voltage (V)
 * @retval NTC resistance (kOhm)
 * @note   Circuit: 10K pullup to VCC, NTC to GND
 *         Formula: R_NTC = R_pullup * V_adc / (V_ref - V_adc)
 */
static float calc_ntc_resistance(float voltage)
{
    // Prevent division by zero
    if (voltage <= 0.0f || voltage >= NTC_VREF) 
        return 1e6f;
    
    return NTC_PULLUP_RES * voltage / (NTC_VREF - voltage);
}

/**
 * @brief  Convert NTC resistance to temperature (with linear interpolation)
 * @param  r_ntc: NTC resistance (kOhm)
 * @retval Temperature (degC)
 */
static float ntc_res_to_temp(float r_ntc)
{
    // Boundary check - higher than maximum resistance (lowest temp)
    if (r_ntc >= ntc_table[0].r_ntc) 
        return ntc_table[0].temp;
    
    // Search and interpolate
    for (int i = 1; i < NTC_TABLE_SIZE; i++)
    {
        if (r_ntc >= ntc_table[i].r_ntc)
        {
            // Linear interpolation
            float t1 = ntc_table[i-1].temp;
            float r1 = ntc_table[i-1].r_ntc;
            float t2 = ntc_table[i].temp;
            float r2 = ntc_table[i].r_ntc;
            
            return t1 + (r_ntc - r1) * (t2 - t1) / (r2 - r1);
        }
    }
    
    // Boundary check - lower than minimum resistance (highest temp)
    return ntc_table[NTC_TABLE_SIZE-1].temp;
}

/**
 * @brief  Update overheat flags with hysteresis
 */
static void update_overheat_flags(void)
{
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        float temp = temp_manager.temperature[i] / 10.0f;  // Convert to degC
        uint8_t current_status = temp_manager.overheat_flag[i];
        
        switch (current_status)
        {
            case TEMP_STATUS_NORMAL:
                if (temp >= TEMP_THRESHOLD_HIGH / 10.0f)
                    temp_manager.overheat_flag[i] = TEMP_STATUS_OVERHEAT;
                else if (temp >= TEMP_THRESHOLD_LOW / 10.0f)
                    temp_manager.overheat_flag[i] = TEMP_STATUS_WARM;
                break;
                
            case TEMP_STATUS_WARM:
                if (temp >= TEMP_THRESHOLD_HIGH / 10.0f)
                    temp_manager.overheat_flag[i] = TEMP_STATUS_OVERHEAT;
                else if (temp < (TEMP_THRESHOLD_LOW - TEMP_HYSTERESIS) / 10.0f)
                    temp_manager.overheat_flag[i] = TEMP_STATUS_NORMAL;
                break;
                
            case TEMP_STATUS_OVERHEAT:
                if (temp < (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS) / 10.0f)
                {
                    if (temp >= TEMP_THRESHOLD_LOW / 10.0f)
                        temp_manager.overheat_flag[i] = TEMP_STATUS_WARM;
                    else
                        temp_manager.overheat_flag[i] = TEMP_STATUS_NORMAL;
                    
                    printf("[Temperature] NTC%d: Temp decreased below 60C\r\n", i+1);
                }
                break;
                
            default:
                temp_manager.overheat_flag[i] = TEMP_STATUS_NORMAL;
                break;
        }
    }
}

/**
 * @brief  Update fan control based on temperature
 */
static void update_fan_control(void)
{
    // Find maximum temperature state
    uint8_t max_state = TEMP_STATUS_NORMAL;
    for (int i = 0; i < TEMP_CHANNEL_COUNT; i++)
    {
        if (temp_manager.overheat_flag[i] > max_state)
            max_state = temp_manager.overheat_flag[i];
    }
    
    // Determine fan speed
    uint8_t new_speed;
    if (max_state == TEMP_STATUS_OVERHEAT || max_state == TEMP_STATUS_WARM)
        new_speed = FAN_SPEED_HIGH;
    else
        new_speed = FAN_SPEED_NORMAL;
    
    // Update if changed
    if (new_speed != temp_manager.fan_speed)
    {
        printf("[Temperature] Fan speed: %d%% -> %d%%\r\n", 
               temp_manager.fan_speed, new_speed);
        Temperature_SetFanSpeed(new_speed);
    }
}

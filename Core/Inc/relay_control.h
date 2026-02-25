/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : relay_control.h
  * @brief          : 缁х數鍣ㄦ帶鍒舵ā鍧楀ご鏂囦欢
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-13
  ******************************************************************************
  * @attention
  *
 * 鏈ā鍧楀疄鐜颁笁閫氶亾缁х數鍣ㄦ帶鍒跺姛鑳斤細
 * 1. 涓夐€氶亾浜掗攣閫昏緫锛堝悓涓€鏃堕棿鍙兘婵€娲讳竴涓€氶亾锛�
 * 2. 鍙岃矾缁х數鍣ㄦ帶鍒讹紙姣忛€氶亾2璺細ON/OFF锛�
 * 3. 500ms璐熻剦鍐茬敓鎴愶紙闈為樆濉烇紝浣庣數骞宠Е鍙戯級
 * 4. 鐘舵€佸弽棣堟娴嬶紙6璺户鐢靛櫒 + 3璺帴瑙﹀櫒锛�
 * 5. 闃叉姈妫€娴嬶紙50ms闂撮殧脳3娆★級
 * 6. K1_EN/K2_EN/K3_EN澶栭儴涓柇鍝嶅簲
 *
 * 纾佷繚鎸佺户鐢靛櫒鎺у埗閫昏緫锛�
 * - 闈欐€佺姸鎬侊細鎵€鏈夋帶鍒跺紩鑴氫繚鎸侀珮鐢靛钩
 * - 瑙﹀彂鍔ㄤ綔锛氳緭鍑�500ms浣庣數骞宠剦鍐�
 * - 鑴夊啿缁撴潫锛氭仮澶嶉珮鐢靛钩锛岀户鐢靛櫒淇濇寔鐘舵€侊紙纾佷繚鎸侊級
  *
  * 纭欢杩炴帴锛�
  * 閫氶亾1锛欿1_1_ON/OFF (PC0/PC1), K1_2_ON/OFF (PA12/PA3)
  *        K1_1_STA (PC4), K1_2_STA (PB1), SW1_STA (PA8)
  *        K1_EN (PB9, 澶栭儴涓柇)
  *
  * 閫氶亾2锛欿2_1_ON/OFF (PC2/PC3), K2_2_ON/OFF (PA4/PA5)
  *        K2_1_STA (PC5), K2_2_STA (PB10), SW2_STA (PC9)
  *        K2_EN (PB8, 澶栭儴涓柇)
  *
  * 閫氶亾3锛欿3_1_ON/OFF (PC7/PC6), K3_2_ON/OFF (PD2/PA7)
  *        K3_1_STA (PB0), K3_2_STA (PB11), SW3_STA (PC8)
  *        K3_EN (PA15, 澶栭儴涓柇)
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
 * @brief 缁х數鍣ㄦ搷浣滅被鍨嬫灇涓�
 */
typedef enum
{
    RELAY_OP_NONE   = 0,  // 鏃犳搷浣�
    RELAY_OP_OPEN   = 1,  // 鎵撳紑锛堟帴閫氾級
    RELAY_OP_CLOSE  = 2   // 鍏抽棴锛堟柇寮€锛�
} RelayOperation_e;

/**
 * @brief 缁х數鍣ㄧ姸鎬佹満鏋氫妇
 */
typedef enum
{
    RELAY_FSM_IDLE      = 0,  // 绌洪棽鐘舵€�
    RELAY_FSM_OPENING   = 1,  // 姝ｅ湪鎵撳紑锛�500ms鑴夊啿涓級
    RELAY_FSM_CLOSING   = 2,  // 姝ｅ湪鍏抽棴锛�500ms鑴夊啿涓級
    RELAY_FSM_OPENED    = 3,  // 宸叉墦寮€
    RELAY_FSM_CLOSED    = 4,  // 宸插叧闂�
    RELAY_FSM_ERROR     = 5   // 閿欒鐘舵€侊紙鐘舵€佸弽棣堝紓甯革級
} RelayFSM_e;

/**
 * @brief EN pin interrupt manager structure
 */
typedef struct {
    volatile uint8_t interrupt_flag;
    uint32_t interrupt_time;
    GPIO_PinState last_pin_state;
    GPIO_PinState pending_pin_state;
    uint8_t verify_step;
} EN_InterruptManager_t;

/**
 * @brief 鍗曡矾缁х數鍣ㄦ帶鍒剁粨鏋�
 */
typedef struct {
    GPIO_TypeDef *on_port;       // ON寮曡剼绔彛
    uint16_t on_pin;             // ON寮曡剼鍙�
    GPIO_TypeDef *off_port;      // OFF寮曡剼绔彛
    uint16_t off_pin;            // OFF寮曡剼鍙�
    GPIO_TypeDef *sta_port;      // STA鐘舵€佸弽棣堝紩鑴氱鍙�
    uint16_t sta_pin;            // STA鐘舵€佸弽棣堝紩鑴氬彿
    RelayFSM_e fsm_state;        // 鐘舵€佹満鐘舵€�
    uint32_t pulse_start_time;   // 鑴夊啿寮€濮嬫椂闂�
    bool expected_state;         // 棰勬湡鐘舵€侊紙true-ON, false-OFF锛�
    uint8_t debounce_count;      // 闃叉姈璁℃暟鍣�
} RelayUnit_t;

/**
 * @brief 閫氶亾鎺у埗缁撴瀯
 */
typedef struct {
    RelayUnit_t relay1;                   // 绗�1璺户鐢靛櫒
    RelayUnit_t relay2;                   // 绗�2璺户鐢靛櫒
    GPIO_TypeDef *sw_sta_port;            // 鎺ヨЕ鍣ㄧ姸鎬佸弽棣堝紩鑴氱鍙�
    uint16_t sw_sta_pin;                  // 鎺ヨЕ鍣ㄧ姸鎬佸弽棣堝紩鑴氬彿
    GPIO_TypeDef *en_port;                // 浣胯兘寮曡剼绔彛
    uint16_t en_pin;                      // 浣胯兘寮曡剼鍙�
    bool is_active;                       // 閫氶亾鏄惁婵€娲�
    RelayOperation_e pending_op;          // 寰呮墽琛屾搷浣�
    EN_InterruptManager_t en_int;         // EN寮曡剼涓柇绠＄悊
} ChannelControl_t;

/**
 * @brief 缁х數鍣ㄧ鐞嗗櫒鏁版嵁缁撴瀯
 */
typedef struct {
    ChannelControl_t channels[3];  // 3涓€氶亾鎺у埗缁撴瀯
    Channel_e active_channel;      // 褰撳墠婵€娲婚€氶亾锛圕HANNEL_NONE琛ㄧず鍏ㄥ叧闂級
    uint32_t last_update_time;     // 涓婃鏇存柊鏃堕棿
    bool initialized;              // 鍒濆鍖栨爣蹇�
} Relay_Manager_t;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief 缁х數鍣ㄦ帶鍒舵椂闂村弬鏁帮紙鍗曚綅锛歮s锛�
 */
#define RELAY_PULSE_WIDTH       500    // 缁х數鍣ㄨ剦鍐插搴︼細500ms
#define RELAY_DEBOUNCE_INTERVAL 50     // 闃叉姈妫€娴嬮棿闅旓細50ms
#define RELAY_DEBOUNCE_TIMES    3      // 闃叉姈妫€娴嬫鏁帮細3娆�`r`n#define EN_PIN_VERIFY_DELAY_MS  10     // EN pin verify delay: 10ms

/**
 * @brief 鐘舵€佸弽棣堟湡鏈涘€硷紙楂樼數骞�=姝ｅ父宸ヤ綔锛�
 */
#define RELAY_STA_EXPECTED_ON   GPIO_PIN_SET    // 缁х數鍣∣N鏃讹紝STA搴斾负楂樼數骞�
#define RELAY_STA_EXPECTED_OFF  GPIO_PIN_RESET  // 缁х數鍣∣FF鏃讹紝STA搴斾负浣庣數骞�

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief 缁х數鍣ㄧ姸鎬佹鏌ュ畯
 */
#define IS_RELAY_BUSY(unit)     ((unit)->fsm_state == RELAY_FSM_OPENING || \
                                 (unit)->fsm_state == RELAY_FSM_CLOSING)
#define IS_RELAY_OPENED(unit)   ((unit)->fsm_state == RELAY_FSM_OPENED)
#define IS_RELAY_CLOSED(unit)   ((unit)->fsm_state == RELAY_FSM_CLOSED)
#define IS_RELAY_ERROR(unit)    ((unit)->fsm_state == RELAY_FSM_ERROR)

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  缁х數鍣ㄦā鍧楀垵濮嬪寲
 * @note   鍒濆鍖朑PIO銆佺姸鎬佹満銆佸叧闂墍鏈夌户鐢靛櫒
 * @retval None
 */
void Relay_Init(void);

/**
 * @brief  鎵撳紑鎸囧畾閫氶亾
 * @param  channel: 閫氶亾缂栧彿锛圕HANNEL_1/2/3锛�
 * @note   浼氳嚜鍔ㄥ叧闂叾浠栭€氶亾锛堜簰閿佹満鍒讹級
 *         鍙戦€�500ms浣庣數骞宠剦鍐插埌ON寮曡剼锛堢淇濇寔缁х數鍣級
 * @retval true-鎴愬姛鍚姩, false-澶辫触锛堝叾浠栭€氶亾蹇欙級
 */
bool Relay_OpenChannel(Channel_e channel);

/**
 * @brief  鍏抽棴鎸囧畾閫氶亾
 * @param  channel: 閫氶亾缂栧彿锛圕HANNEL_1/2/3锛�
 * @note   鍙戦€�500ms浣庣數骞宠剦鍐插埌OFF寮曡剼锛堢淇濇寔缁х數鍣級
 * @retval true-鎴愬姛鍚姩, false-澶辫触锛堥€氶亾蹇欙級
 */
bool Relay_CloseChannel(Channel_e channel);

/**
 * @brief  鍏抽棴鎵€鏈夐€氶亾
 * @note   鐢ㄤ簬绱ф€ュ仠姝㈡垨绯荤粺澶嶄綅
 * @retval None
 */
void Relay_CloseAll(void);

/**
 * @brief  上电复位专用：无视 is_active 标志，强制对三路全部发送 OFF 脉冲
 * @note   用于自检 LOGO 阶段，确保磁保持继电器回到断开状态
 * @retval None
 */
void Relay_ForceCloseAll(void);

/**
 * @brief  鑾峰彇褰撳墠婵€娲婚€氶亾
 * @retval 褰撳墠婵€娲荤殑閫氶亾缂栧彿锛圕HANNEL_NONE琛ㄧず鍏ㄥ叧闂級
 */
Channel_e Relay_GetActiveChannel(void);

/**
 * @brief  妫€鏌ラ€氶亾鏄惁蹇欑
 * @param  channel: 閫氶亾缂栧彿
 * @retval true-蹇欑锛堟鍦ㄦ墽琛屾搷浣滐級, false-绌洪棽
 */
bool Relay_IsChannelBusy(Channel_e channel);

/**
 * @brief  妫€鏌ラ€氶亾鐘舵€佸弽棣堟槸鍚︽甯�
 * @param  channel: 閫氶亾缂栧彿
 * @retval true-姝ｅ父, false-寮傚父
 */
bool Relay_CheckChannelFeedback(Channel_e channel);

/**
 * @brief  鏇存柊缁х數鍣ㄧ姸鎬侊紙闈為樆濉烇級
 * @note   瀹氭椂璋冪敤姝ゅ嚱鏁版洿鏂扮姸鎬佹満銆佹娴嬬姸鎬佸弽棣�
 *         寤鸿璋冪敤棰戠巼锛�20-50Hz锛�20-50ms锛�
 * @retval None
 */
void Relay_Update(void);

/**
 * @brief  鎵撳嵃缁х數鍣ㄧ姸鎬侊紙璋冭瘯鐢級
 * @note   閫氳繃涓插彛杈撳嚭褰撳墠閫氶亾鐘舵€併€佺户鐢靛櫒鐘舵€佺瓑淇℃伅
 * @retval None
 */
void Relay_PrintStatus(void);

/**
 * @brief  K1_EN涓柇鍥炶皟锛堝閮ㄨ皟鐢級
 * @note   鐢眘tm32f1xx_it.c涓殑HAL_GPIO_EXTI_Callback璋冪敤
 * @retval None
 */
void Relay_K1_EN_ISR(void);

/**
 * @brief  K2_EN涓柇鍥炶皟锛堝閮ㄨ皟鐢級
 * @note   鐢眘tm32f1xx_it.c涓殑HAL_GPIO_EXTI_Callback璋冪敤
 * @retval None
 */
void Relay_K2_EN_ISR(void);

/**
 * @brief  K3_EN涓柇鍥炶皟锛堝閮ㄨ皟鐢級
 * @note   鐢眘tm32f1xx_it.c涓殑HAL_GPIO_EXTI_Callback璋冪敤
 * @retval None
 */
void Relay_K3_EN_ISR(void);

/**
 * @brief  鑾峰彇缁х數鍣ㄧ鐞嗗櫒鎸囬拡锛堣皟璇曠敤锛�
 * @retval 缁х數鍣ㄧ鐞嗗櫒鏁版嵁缁撴瀯鎸囬拡
 */
const Relay_Manager_t* Relay_GetManager(void);

#ifdef __cplusplus
}
#endif

#endif /* __RELAY_CONTROL_H */

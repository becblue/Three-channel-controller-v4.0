/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : self_test.c
  * @brief          : 系统自检模块实现
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 自检流程时序（共约 5.5 秒）：
  *   [0ms]     LOGO 显示 2000ms
  *   [2000ms]  Step1 期望状态识别 500ms（进度 0%→25%）
  *   [2500ms]  Step2 继电器纠错 800ms（进度 25%→50%，含 500ms 脉冲等待）
  *   [3300ms]  Step3 接触器检查 500ms（进度 50%→75%）
  *   [3800ms]  Step4 温度检测 700ms（进度 75%→100%）
  *   [4500ms]  结果显示 PASS/FAIL 1500ms，结束
  *
  * 关键设计原则：
  *   1. SelfTest_Update() 仅推进自检状态机，不调用 Relay_Update()。
  *      Relay_Update() 由主循环 20ms 任务持续驱动，确保 Step2 纠错脉冲正常执行。
  *   2. Step2 在进入时立即发起纠错脉冲，在 800ms 超时后验证 STA 结果。
  *   3. 每步检测动作通过 step_action_done 标志保证只执行一次。
  *   4. OLED 进度条仅在百分比变化时刷新，避免闪烁。
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "self_test.h"
#include "oled_display.h"
#include "relay_control.h"
#include "temperature.h"
#include "safety_monitor.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/* 各阶段持续时间（ms） */
#define ST_LOGO_MS          TIME_LOGO_DISPLAY   ///< LOGO 显示时长：2000ms
#define ST_STEP1_MS         500U                ///< Step1 持续时长
#define ST_STEP2_MS         800U                ///< Step2 持续时长（500ms脉冲 + 300ms稳定）
#define ST_STEP3_MS         500U                ///< Step3 持续时长
#define ST_STEP4_MS         700U                ///< Step4 持续时长
#define ST_RESULT_MS        1500U               ///< PASS/FAIL 结果停留时长

/* 进度条各步骤起始基准（单位：%，共 100%） */
#define PROG_STEP1_BASE     0U
#define PROG_STEP2_BASE     25U
#define PROG_STEP3_BASE     50U
#define PROG_STEP4_BASE     75U

/* Private types -------------------------------------------------------------*/

/**
 * @brief 自检上下文（最小内部状态）
 */
typedef struct
{
    SelfTest_State_e state;             ///< 当前自检状态
    uint32_t         step_start_ms;     ///< 当前状态进入时的时间戳
    bool             step_action_done;  ///< 当前步骤主要动作是否已执行
    bool             step_pass[4];      ///< Step1~4 各步结果（下标 0~3）
    uint8_t          expected_open_ch;  ///< Step1 识别出的期望打开通道（0=无，1~3）
    uint8_t          last_progress;     ///< 上次 OLED 显示进度（防重复刷新，0xFF=强制刷新）
    bool             initialized;       ///< 模块是否已启动
    bool             passed;            ///< 最终结果
} SelfTest_Ctx_t;

/* Private variables ---------------------------------------------------------*/

static SelfTest_Ctx_t s_ctx;

/**
 * @brief 三通道引脚查找表（按通道索引 0~2）
 */
static const struct
{
    GPIO_TypeDef *en_port;        ///< K_EN 端口
    uint16_t      en_pin;         ///< K_EN 引脚
    GPIO_TypeDef *k1_sta_port;    ///< K_1_STA 端口
    uint16_t      k1_sta_pin;     ///< K_1_STA 引脚
    GPIO_TypeDef *sw_sta_port;    ///< SW_STA 端口
    uint16_t      sw_sta_pin;     ///< SW_STA 引脚
    Channel_e     channel;        ///< 通道枚举值
} s_ch_map[3] =
{
    { K1_EN_GPIO_Port, K1_EN_Pin, K1_1_STA_GPIO_Port, K1_1_STA_Pin, SW1_STA_GPIO_Port, SW1_STA_Pin, CHANNEL_1 },
    { K2_EN_GPIO_Port, K2_EN_Pin, K2_1_STA_GPIO_Port, K2_1_STA_Pin, SW2_STA_GPIO_Port, SW2_STA_Pin, CHANNEL_2 },
    { K3_EN_GPIO_Port, K3_EN_Pin, K3_1_STA_GPIO_Port, K3_1_STA_Pin, SW3_STA_GPIO_Port, SW3_STA_Pin, CHANNEL_3 },
};

/* Private function prototypes -----------------------------------------------*/
static void    st_enter_state(SelfTest_State_e new_state);
static void    st_exec_step1(void);
static void    st_exec_step2(void);
static bool    st_verify_step2(void);
static void    st_exec_step3(void);
static void    st_exec_step4(void);
static uint8_t st_calc_progress(SelfTest_State_e state, uint32_t elapsed_ms);
static void    st_update_oled_progress(SelfTest_State_e state, uint32_t elapsed_ms);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  启动系统自检
 */
void SelfTest_Start(void)
{
    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.last_progress = 0xFFU; /* 强制第一次进度条刷新 */
    s_ctx.initialized   = true;
    printf("[SelfTest] Starting self-test sequence...\r\n");
    st_enter_state(SELF_TEST_LOGO);
}

/**
 * @brief  查询自检是否仍在运行
 */
uint8_t SelfTest_IsRunning(void)
{
    if (!s_ctx.initialized)
    {
        return 0U;
    }
    /* IDLE / PASS（结束后）/ FAIL（结束后）均视为不在运行 */
    return (s_ctx.state != SELF_TEST_IDLE) ? 1U : 0U;
}

/**
 * @brief  查询自检是否通过
 */
uint8_t SelfTest_IsPassed(void)
{
    return s_ctx.passed ? 1U : 0U;
}

/**
 * @brief  自检状态机更新（非阻塞，每 20ms 调用）
 */
void SelfTest_Update(void)
{
    if (!s_ctx.initialized || s_ctx.state == SELF_TEST_IDLE)
    {
        return;
    }

    uint32_t now     = HAL_GetTick();
    uint32_t elapsed = now - s_ctx.step_start_ms;

    switch (s_ctx.state)
    {
        /* ------------------------------------------------------------------ */
        case SELF_TEST_LOGO:
            if (elapsed >= ST_LOGO_MS)
            {
                st_enter_state(SELF_TEST_STEP1_STATE_IDENTIFY);
            }
            break;

        /* ------------------------------------------------------------------ */
        case SELF_TEST_STEP1_STATE_IDENTIFY:
            st_update_oled_progress(s_ctx.state, elapsed);
            if (!s_ctx.step_action_done)
            {
                s_ctx.step_action_done = true;
                st_exec_step1();
            }
            if (elapsed >= ST_STEP1_MS)
            {
                st_enter_state(SELF_TEST_STEP2_RELAY_CHECK);
            }
            break;

        /* ------------------------------------------------------------------ */
        case SELF_TEST_STEP2_RELAY_CHECK:
            st_update_oled_progress(s_ctx.state, elapsed);
            if (!s_ctx.step_action_done)
            {
                /* 进入 Step2 立即发起纠错脉冲（由主循环 Relay_Update 执行） */
                s_ctx.step_action_done = true;
                st_exec_step2();
            }
            if (elapsed >= ST_STEP2_MS)
            {
                /* 脉冲应已完成（500ms），再等 300ms 后验证 STA */
                s_ctx.step_pass[1] = st_verify_step2();
                st_enter_state(SELF_TEST_STEP3_SWITCH_CHECK);
            }
            break;

        /* ------------------------------------------------------------------ */
        case SELF_TEST_STEP3_SWITCH_CHECK:
            st_update_oled_progress(s_ctx.state, elapsed);
            if (!s_ctx.step_action_done)
            {
                s_ctx.step_action_done = true;
                st_exec_step3();
            }
            if (elapsed >= ST_STEP3_MS)
            {
                st_enter_state(SELF_TEST_STEP4_TEMP_CHECK);
            }
            break;

        /* ------------------------------------------------------------------ */
        case SELF_TEST_STEP4_TEMP_CHECK:
            st_update_oled_progress(s_ctx.state, elapsed);
            if (!s_ctx.step_action_done)
            {
                s_ctx.step_action_done = true;
                st_exec_step4();
            }
            if (elapsed >= ST_STEP4_MS)
            {
                bool all_pass = s_ctx.step_pass[0] && s_ctx.step_pass[1] &&
                                s_ctx.step_pass[2] && s_ctx.step_pass[3];
                st_enter_state(all_pass ? SELF_TEST_PASS : SELF_TEST_FAIL);
            }
            break;

        /* ------------------------------------------------------------------ */
        case SELF_TEST_PASS:
            if (!s_ctx.step_action_done)
            {
                s_ctx.step_action_done = true;
                s_ctx.passed           = true;
                Safety_ClearSelfTestError();
                /* OLED 显示 PASS */
                OLED_Clear();
                OLED_ShowString(20U, 2U, "Self Test", OLED_FONT_8X16);
                OLED_ShowString(40U, 5U, "PASS",      OLED_FONT_8X16);
                OLED_Refresh();
                printf("[SelfTest] *** SELF TEST PASS ***\r\n");
                SelfTest_PrintResult();
            }
            if (elapsed >= ST_RESULT_MS)
            {
                s_ctx.state = SELF_TEST_IDLE;
            }
            break;

        /* ------------------------------------------------------------------ */
        case SELF_TEST_FAIL:
            if (!s_ctx.step_action_done)
            {
                s_ctx.step_action_done = true;
                s_ctx.passed           = false;
                Safety_SetSelfTestError();
                /* OLED 显示 FAIL */
                OLED_Clear();
                OLED_ShowString(20U, 2U, "Self Test", OLED_FONT_8X16);
                OLED_ShowString(40U, 5U, "FAIL",      OLED_FONT_8X16);
                OLED_Refresh();
                printf("[SelfTest] *** SELF TEST FAIL ***\r\n");
                SelfTest_PrintResult();
            }
            if (elapsed >= ST_RESULT_MS)
            {
                s_ctx.state = SELF_TEST_IDLE;
            }
            break;

        default:
            break;
    }
}

/**
 * @brief  打印自检各步骤结果（调试用）
 */
void SelfTest_PrintResult(void)
{
    printf("[SelfTest] Result: %s\r\n",         s_ctx.passed        ? "PASS" : "FAIL");
    printf("  Step1 State Identify : %s\r\n",   s_ctx.step_pass[0]  ? "PASS" : "FAIL");
    printf("  Step2 Relay Correct  : %s\r\n",   s_ctx.step_pass[1]  ? "PASS" : "FAIL");
    printf("  Step3 Switch Check   : %s\r\n",   s_ctx.step_pass[2]  ? "PASS" : "FAIL");
    printf("  Step4 Temp Check     : %s\r\n",   s_ctx.step_pass[3]  ? "PASS" : "FAIL");
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  切换到新状态，重置计时器和动作标志，并更新 OLED 界面
 */
static void st_enter_state(SelfTest_State_e new_state)
{
    s_ctx.state            = new_state;
    s_ctx.step_start_ms    = HAL_GetTick();
    s_ctx.step_action_done = false;

    switch (new_state)
    {
        case SELF_TEST_LOGO:
            OLED_Clear();
            OLED_ShowLogo();
            OLED_Refresh();
            printf("[SelfTest] Showing LOGO (%ums)...\r\n", (unsigned int)ST_LOGO_MS);
            break;

        case SELF_TEST_STEP1_STATE_IDENTIFY:
            /* 绘制自检基础界面：标题 + 步骤名称 + 初始进度条 */
            OLED_Clear();
            OLED_ShowString(0U, 0U, "Self Testing...",   OLED_FONT_6X8);
            OLED_ShowString(0U, 6U, "Step1:State Check", OLED_FONT_6X8);
            OLED_ShowProgress(0U);
            OLED_Refresh();
            s_ctx.last_progress = 0U;
            printf("[SelfTest] Step1: State identify\r\n");
            break;

        case SELF_TEST_STEP2_RELAY_CHECK:
            /* 只更新步骤名称行和进度条，保留标题行 */
            OLED_ShowString(0U, 6U, "Step2:Relay Check", OLED_FONT_6X8);
            OLED_ShowProgress(PROG_STEP2_BASE);
            OLED_Refresh();
            s_ctx.last_progress = (uint8_t)PROG_STEP2_BASE;
            printf("[SelfTest] Step2: Relay correction\r\n");
            break;

        case SELF_TEST_STEP3_SWITCH_CHECK:
            OLED_ShowString(0U, 6U, "Step3:SwitchCheck", OLED_FONT_6X8);
            OLED_ShowProgress(PROG_STEP3_BASE);
            OLED_Refresh();
            s_ctx.last_progress = (uint8_t)PROG_STEP3_BASE;
            printf("[SelfTest] Step3: Switch check\r\n");
            break;

        case SELF_TEST_STEP4_TEMP_CHECK:
            OLED_ShowString(0U, 6U, "Step4:Temp Check ", OLED_FONT_6X8);
            OLED_ShowProgress(PROG_STEP4_BASE);
            OLED_Refresh();
            s_ctx.last_progress = (uint8_t)PROG_STEP4_BASE;
            printf("[SelfTest] Step4: Temperature check\r\n");
            break;

        default:
            break;
    }
}

/**
 * @brief  根据当前状态和已耗时，计算进度百分比（0~100）
 */
static uint8_t st_calc_progress(SelfTest_State_e state, uint32_t elapsed_ms)
{
    uint8_t  base = 0U;
    uint32_t dur  = 1U;

    switch (state)
    {
        case SELF_TEST_STEP1_STATE_IDENTIFY: base = PROG_STEP1_BASE; dur = ST_STEP1_MS; break;
        case SELF_TEST_STEP2_RELAY_CHECK:    base = PROG_STEP2_BASE; dur = ST_STEP2_MS; break;
        case SELF_TEST_STEP3_SWITCH_CHECK:   base = PROG_STEP3_BASE; dur = ST_STEP3_MS; break;
        case SELF_TEST_STEP4_TEMP_CHECK:     base = PROG_STEP4_BASE; dur = ST_STEP4_MS; break;
        default: return 0U;
    }

    if (elapsed_ms >= dur)
    {
        return base + 25U;
    }
    return (uint8_t)(base + (elapsed_ms * 25U / dur));
}

/**
 * @brief  若进度百分比有变化则刷新 OLED 进度条（防止频繁全屏刷新）
 */
static void st_update_oled_progress(SelfTest_State_e state, uint32_t elapsed_ms)
{
    uint8_t p = st_calc_progress(state, elapsed_ms);
    if (p == s_ctx.last_progress)
    {
        return;
    }
    s_ctx.last_progress = p;
    OLED_ShowProgress(p);
    OLED_Refresh();
}

/**
 * @brief  Step1：读取三路 K_EN 识别期望打开通道，检测使能冲突
 *
 * 规则：K_EN = LOW → 通道期望打开（继电器吸合，STA HIGH）
 *       K_EN = HIGH → 通道期望关闭（继电器断开，STA LOW）
 */
static void st_exec_step1(void)
{
    uint8_t low_cnt            = 0U;
    s_ctx.expected_open_ch     = 0U;

    for (uint8_t i = 0U; i < 3U; i++)
    {
        if (HAL_GPIO_ReadPin(s_ch_map[i].en_port, s_ch_map[i].en_pin) == GPIO_PIN_RESET)
        {
            low_cnt++;
            s_ctx.expected_open_ch = (uint8_t)(i + 1U); /* 暂记，若 >1 则无效 */
        }
    }

    if (low_cnt > 1U)
    {
        /* 多路同时使能：使能冲突 */
        s_ctx.step_pass[0]     = false;
        s_ctx.expected_open_ch = 0U;
        printf("[SelfTest] Step1 FAIL: Enable conflict (%d channels enabled)\r\n",
               (int)low_cnt);
    }
    else
    {
        s_ctx.step_pass[0] = true;
        printf("[SelfTest] Step1 PASS: Expected open CH=%d\r\n",
               (int)s_ctx.expected_open_ch);
    }
}

/**
 * @brief  Step2：针对 STA 与期望不符的通道发起纠错脉冲
 *
 * 策略：先处理所有"应关闭"通道（Relay_CloseChannel，会立即释放互锁），
 *       再处理"应打开"通道（最多一个，互锁此时已解除）。
 * 验证：由 st_verify_step2() 在 ST_STEP2_MS 超时后调用。
 */
static void st_exec_step2(void)
{
    if (!s_ctx.step_pass[0])
    {
        /* Step1 失败（使能冲突），无法安全纠错 */
        printf("[SelfTest] Step2 SKIP: Step1 failed, no correction\r\n");
        return;
    }

    /* 第一轮：关闭所有应关闭但 STA 仍为 HIGH 的通道 */
    for (uint8_t i = 0U; i < 3U; i++)
    {
        bool should_open = (HAL_GPIO_ReadPin(s_ch_map[i].en_port, s_ch_map[i].en_pin)
                            == GPIO_PIN_RESET);
        bool sta_high    = (HAL_GPIO_ReadPin(s_ch_map[i].k1_sta_port, s_ch_map[i].k1_sta_pin)
                            == GPIO_PIN_SET);

        if (!should_open && sta_high && !Relay_IsChannelBusy(s_ch_map[i].channel))
        {
            printf("[SelfTest] Step2: CH%d should be CLOSED, sending OFF pulse\r\n",
                   (int)(i + 1U));
            Relay_CloseChannel(s_ch_map[i].channel);
        }
    }

    /* 第二轮：打开应打开但 STA 仍为 LOW 的通道（互锁已在第一轮中解除） */
    for (uint8_t i = 0U; i < 3U; i++)
    {
        bool should_open = (HAL_GPIO_ReadPin(s_ch_map[i].en_port, s_ch_map[i].en_pin)
                            == GPIO_PIN_RESET);
        bool sta_high    = (HAL_GPIO_ReadPin(s_ch_map[i].k1_sta_port, s_ch_map[i].k1_sta_pin)
                            == GPIO_PIN_SET);

        if (should_open && !sta_high && !Relay_IsChannelBusy(s_ch_map[i].channel))
        {
            printf("[SelfTest] Step2: CH%d should be OPEN, sending ON pulse\r\n",
                   (int)(i + 1U));
            Relay_OpenChannel(s_ch_map[i].channel);
        }
    }
}

/**
 * @brief  Step2 结束时验证所有通道 K_1_STA 是否与 K_EN 期望一致
 * @retval true = 全部一致（通过）；false = 存在不匹配（失败）
 */
static bool st_verify_step2(void)
{
    if (!s_ctx.step_pass[0])
    {
        return false;
    }

    bool pass = true;
    for (uint8_t i = 0U; i < 3U; i++)
    {
        bool should_open = (HAL_GPIO_ReadPin(s_ch_map[i].en_port, s_ch_map[i].en_pin)
                            == GPIO_PIN_RESET);
        bool sta_high    = (HAL_GPIO_ReadPin(s_ch_map[i].k1_sta_port, s_ch_map[i].k1_sta_pin)
                            == GPIO_PIN_SET);

        if (should_open != sta_high)
        {
            printf("[SelfTest] Step2 FAIL: CH%d K_1_STA mismatch (expected %s, got %s)\r\n",
                   (int)(i + 1U),
                   should_open ? "HIGH" : "LOW",
                   sta_high    ? "HIGH" : "LOW");
            pass = false;
        }
    }

    if (pass)
    {
        printf("[SelfTest] Step2 PASS: All relay states correct\r\n");
    }
    return pass;
}

/**
 * @brief  Step3：检查三路 SW_STA（接触器）是否与 K_EN 期望一致
 * @note   接触器无法通过软件纠错，不匹配直接标记失败
 */
static void st_exec_step3(void)
{
    bool pass = true;
    for (uint8_t i = 0U; i < 3U; i++)
    {
        bool should_open = (HAL_GPIO_ReadPin(s_ch_map[i].en_port, s_ch_map[i].en_pin)
                            == GPIO_PIN_RESET);
        bool sw_high     = (HAL_GPIO_ReadPin(s_ch_map[i].sw_sta_port, s_ch_map[i].sw_sta_pin)
                            == GPIO_PIN_SET);

        if (should_open != sw_high)
        {
            printf("[SelfTest] Step3 FAIL: CH%d SW_STA mismatch (expected %s, got %s)\r\n",
                   (int)(i + 1U),
                   should_open ? "HIGH" : "LOW",
                   sw_high     ? "HIGH" : "LOW");
            pass = false;
        }
    }

    s_ctx.step_pass[2] = pass;
    if (pass)
    {
        printf("[SelfTest] Step3 PASS: All switch states correct\r\n");
    }
}

/**
 * @brief  Step4：触发一次温度更新，检查三路 NTC 是否过温（≥60°C）
 */
static void st_exec_step4(void)
{
    Temperature_Update(); /* 刷新温度数据（自检期间 1s 任务暂停，需手动触发） */

    bool pass = true;
    for (uint8_t i = 0U; i < 3U; i++)
    {
        if (Temperature_GetOverheatFlag(i) == TEMP_STATUS_OVERHEAT)
        {
            printf("[SelfTest] Step4 FAIL: NTC%d overheat detected\r\n", (int)(i + 1U));
            pass = false;
        }
    }

    s_ctx.step_pass[3] = pass;
    if (pass)
    {
        printf("[SelfTest] Step4 PASS: Temperature normal\r\n");
    }
}

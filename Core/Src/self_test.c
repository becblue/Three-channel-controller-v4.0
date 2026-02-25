/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : self_test.c
  * @brief          : 缁崵绮洪懛顏咁梾濡€虫健鐎圭偟骞�
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 閼奉亝顥呭ù浣衡柤閺冭泛绨敍鍫濆彙缁撅拷 5.5 缁夋帪绱氶敍锟�
  *   [0ms]     LOGO 閺勫墽銇� 2000ms
  *   [2000ms]  Step1 閺堢喐婀滈悩鑸碘偓浣界槕閸掞拷 500ms閿涘牐绻樻惔锟� 0%閳拷25%閿涳拷
  *   [2500ms]  Step2 缂佈呮暩閸ｃ劎绫傞柨锟� 800ms閿涘牐绻樻惔锟� 25%閳拷50%閿涘苯鎯� 500ms 閼村鍟跨粵澶婄窡閿涳拷
  *   [3300ms]  Step3 閹恒儴袝閸ｃ劍顥呴弻锟� 500ms閿涘牐绻樻惔锟� 50%閳拷75%閿涳拷
  *   [3800ms]  Step4 濞撯晛瀹冲Λ鈧ù锟� 700ms閿涘牐绻樻惔锟� 75%閳拷100%閿涳拷
  *   [4500ms]  缂佹挻鐏夐弰鍓с仛 PASS/FAIL 1500ms閿涘瞼绮ㄩ弶锟�
  *
  * 閸忔娊鏁拋鎹愵吀閸樼喎鍨敍锟�
  *   1. SelfTest_Update() 娴犲懏甯规潻娑滃殰濡偓閻樿埖鈧焦婧€閿涘奔绗夌拫鍐暏 Relay_Update()閵嗭拷
  *      Relay_Update() 閻㈠彉瀵屽顏嗗箚 20ms 娴犺濮熼幐浣虹敾妞瑰崬濮╅敍宀€鈥樻穱锟� Step2 缁剧娀鏁婇懘澶婂暱濮濓絽鐖堕幍褑顢戦妴锟�
  *   2. Step2 閸︺劏绻橀崗銉︽缁斿宓嗛崣鎴ｆ崳缁剧娀鏁婇懘澶婂暱閿涘苯婀� 800ms 鐡掑懏妞傞崥搴ㄧ崣鐠囷拷 STA 缂佹挻鐏夐妴锟�
  *   3. 濮ｅ繑顒炲Λ鈧ù瀣З娴ｆ粓鈧俺绻� step_action_done 閺嶅洤绻旀穱婵婄槈閸欘亝澧界悰灞肩濞喡扳偓锟�
  *   4. OLED 鏉╂稑瀹抽弶鈥茬矌閸︺劎娅ㄩ崚鍡樼槷閸欐ê瀵查弮璺哄煕閺傚府绱濋柆鍨帳闂傤亞鍎婇妴锟�
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

/* 閸氬嫰妯佸▓鍨瘮缂侇厽妞傞梻杈剧礄ms閿涳拷 */
#define ST_LOGO_MS          TIME_LOGO_DISPLAY   ///< LOGO 閺勫墽銇氶弮鍫曟毐閿涳拷2000ms
#define ST_STEP1_MS         500U                ///< Step1 閹镐胶鐢婚弮鍫曟毐
#define ST_STEP2_MS         800U                ///< Step2 閹镐胶鐢婚弮鍫曟毐閿涳拷500ms閼村鍟� + 300ms缁嬪啿鐣鹃敍锟�
#define ST_STEP3_MS         500U                ///< Step3 閹镐胶鐢婚弮鍫曟毐
#define ST_STEP4_MS         700U                ///< Step4 閹镐胶鐢婚弮鍫曟毐
#define ST_RESULT_MS        1500U               ///< PASS/FAIL 缂佹挻鐏夐崑婊呮殌閺冨爼鏆�

/* 鏉╂稑瀹抽弶鈥虫倗濮濄儵顎冪挧宄邦潗閸╁搫鍣敍鍫濆礋娴ｅ稄绱�%閿涘苯鍙� 100%閿涳拷 */
#define PROG_STEP1_BASE     0U
#define PROG_STEP2_BASE     25U
#define PROG_STEP3_BASE     50U
#define PROG_STEP4_BASE     75U

/* Private types -------------------------------------------------------------*/

/**
 * @brief 閼奉亝顥呮稉濠佺瑓閺傚浄绱欓張鈧亸蹇撳敶闁劎濮搁幀渚婄礆
 */
typedef struct
{
    SelfTest_State_e state;             ///< 瑜版挸澧犻懛顏咁梾閻樿埖鈧拷
    uint32_t         step_start_ms;     ///< 瑜版挸澧犻悩鑸碘偓浣界箻閸忋儲妞傞惃鍕闂傚瓨鍩�
    bool             step_action_done;  ///< 瑜版挸澧犲銉╊€冩稉鏄忣洣閸斻劋缍旈弰顖氭儊瀹稿弶澧界悰锟�
    bool             step_pass[4];      ///< Step1~4 閸氬嫭顒炵紒鎾寸亯閿涘牅绗呴弽锟� 0~3閿涳拷
    uint8_t          expected_open_ch;  ///< Step1 鐠囧棗鍩嗛崙铏规畱閺堢喐婀滈幍鎾崇磻闁岸浜鹃敍锟�0=閺冪媴绱�1~3閿涳拷
    uint8_t          last_progress;     ///< 娑撳﹥顐� OLED 閺勫墽銇氭潻娑樺閿涘牓妲婚柌宥咁槻閸掗攱鏌婇敍锟�0xFF=瀵搫鍩楅崚閿嬫煀閿涳拷
    bool             initialized;       ///< 濡€虫健閺勵垰鎯佸鎻掓儙閸旓拷
    bool             passed;            ///< 閺堚偓缂佸牏绮ㄩ弸锟�
} SelfTest_Ctx_t;

/* Private variables ---------------------------------------------------------*/

static SelfTest_Ctx_t s_ctx;

/**
 * @brief 娑撳鈧岸浜惧鏇″壖閺屻儲澹樼悰顭掔礄閹稿鈧岸浜剧槐銏犵穿 0~2閿涳拷
 */
static const struct
{
    GPIO_TypeDef *en_port;        ///< K_EN 缁旑垰褰�
    uint16_t      en_pin;         ///< K_EN 瀵洝鍓�
    GPIO_TypeDef *k1_sta_port;    ///< K_1_STA 缁旑垰褰�
    uint16_t      k1_sta_pin;     ///< K_1_STA 瀵洝鍓�
    GPIO_TypeDef *sw_sta_port;    ///< SW_STA 缁旑垰褰�
    uint16_t      sw_sta_pin;     ///< SW_STA 瀵洝鍓�
    Channel_e     channel;        ///< 闁岸浜鹃弸姘閸婏拷
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
 * @brief  閸氼垰濮╃化鑽ょ埠閼奉亝顥�
 */
void SelfTest_Start(void)
{
    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.last_progress = 0xFFU; /* 瀵搫鍩楃粭顑跨濞喡ょ箻鎼达附娼崚閿嬫煀 */
    s_ctx.initialized   = true;
    printf("[SelfTest] Starting self-test sequence...\r\n");
    st_enter_state(SELF_TEST_LOGO);
}

/**
 * @brief  閺屻儴顕楅懛顏咁梾閺勵垰鎯佹禒宥呮躬鏉╂劘顢�
 */
uint8_t SelfTest_IsRunning(void)
{
    if (!s_ctx.initialized)
    {
        return 0U;
    }
    /* IDLE / PASS閿涘牏绮ㄩ弶鐔锋倵閿涳拷/ FAIL閿涘牏绮ㄩ弶鐔锋倵閿涘娼庣憴鍡曡礋娑撳秴婀潻鎰攽 */
    return (s_ctx.state != SELF_TEST_IDLE) ? 1U : 0U;
}

/**
 * @brief  閺屻儴顕楅懛顏咁梾閺勵垰鎯侀柅姘崇箖
 */
uint8_t SelfTest_IsPassed(void)
{
    return s_ctx.passed ? 1U : 0U;
}

/**
 * @brief  閼奉亝顥呴悩鑸碘偓浣规簚閺囧瓨鏌婇敍鍫ユ姜闂冭顢ｉ敍灞剧槨 20ms 鐠嬪啰鏁ら敍锟�
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
                /* 鏉╂稑鍙� Step2 缁斿宓嗛崣鎴ｆ崳缁剧娀鏁婇懘澶婂暱閿涘牏鏁辨稉璇叉儕閻滐拷 Relay_Update 閹笛嗩攽閿涳拷 */
                s_ctx.step_action_done = true;
                st_exec_step2();
            }
            if (elapsed >= ST_STEP2_MS)
            {
                /* 閼村鍟挎惔鏂垮嚒鐎瑰本鍨氶敍锟�500ms閿涘绱濋崘宥囩搼 300ms 閸氬酣鐛欑拠锟� STA */
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
                /* OLED 閺勫墽銇� PASS */
                OLED_Clear();
                OLED_ShowString(28U, 0U, "SELF-TEST", OLED_FONT_8X16);
                OLED_ShowString(48U, 4U, "PASS",      OLED_FONT_8X16);
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
                /* OLED 閺勫墽銇� FAIL */
                OLED_Clear();
                OLED_ShowString(28U, 0U, "SELF-TEST", OLED_FONT_8X16);
                OLED_ShowString(48U, 4U, "FAIL",      OLED_FONT_8X16);
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
 * @brief  閹垫挸宓冮懛顏咁梾閸氬嫭顒炴銈囩波閺嬫粣绱欑拫鍐槸閻㈩煉绱�
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
 * @brief  閸掑洦宕查崚鐗堟煀閻樿埖鈧緤绱濋柌宥囩枂鐠佲剝妞傞崳銊ユ嫲閸斻劋缍旈弽鍥х箶閿涘苯鑻熼弴瀛樻煀 OLED 閻ｅ矂娼�
 */
static void st_enter_state(SelfTest_State_e new_state)
{
    s_ctx.state            = new_state;
    s_ctx.step_start_ms    = HAL_GetTick();
    s_ctx.step_action_done = false;

    switch (new_state)
    {
        case SELF_TEST_LOGO:
            /* OLED_ShowLogo() 閸愬懘鍎村鎻掑瘶閸氾拷 OLED_Clear() 閸滐拷 OLED_Refresh() */
            OLED_ShowLogo();
            printf("[SelfTest] Showing LOGO (%ums)...\r\n", (unsigned int)ST_LOGO_MS);
            break;

        case SELF_TEST_STEP1_STATE_IDENTIFY:
            /*
             * 閻ｅ矂娼�2鐢啫鐪敍锟�
             *   row0 (y=0~7) : 閺嶅洭顣� "Self Testing..."
             *   row1~4(y=8~39): minyerLOGO閿涘牆鐪虫稉顓ㄧ礆
             *   row5~6(y=40~55): 鏉╂稑瀹抽弶锟� + 閻ф儳鍨庡В锟�
             *   row7 (y=56~63): 濮濄儵顎冮崥锟�
             */
            OLED_Clear();
            OLED_ShowString(0U, 0U, "Self Testing...", OLED_FONT_6X8);
            OLED_ShowMinyerLogo();
            OLED_ShowProgress(0U);
            OLED_ShowString(4U, 7U, "Step1:State Check", OLED_FONT_6X8);
            OLED_Refresh();
            s_ctx.last_progress = 0U;
            printf("[SelfTest] Step1: State identify\r\n");
            break;

        case SELF_TEST_STEP2_RELAY_CHECK:
            /* 閸欘亜鍩涢弬锟� row7閿涘牊顒炴銈呮倳閿涘鎷版潻娑樺閸栫尨绱濋弽鍥暯娑擄拷 LOGO 娣囨繄鏆€ */
            OLED_ShowString(4U, 7U, "Step2:Relay Check", OLED_FONT_6X8);
            OLED_ShowProgress(PROG_STEP2_BASE);
            OLED_Refresh();
            s_ctx.last_progress = (uint8_t)PROG_STEP2_BASE;
            printf("[SelfTest] Step2: Relay correction\r\n");
            break;

        case SELF_TEST_STEP3_SWITCH_CHECK:
            OLED_ShowString(4U, 7U, "Step3:SwitchCheck", OLED_FONT_6X8);
            OLED_ShowProgress(PROG_STEP3_BASE);
            OLED_Refresh();
            s_ctx.last_progress = (uint8_t)PROG_STEP3_BASE;
            printf("[SelfTest] Step3: Switch check\r\n");
            break;

        case SELF_TEST_STEP4_TEMP_CHECK:
            OLED_ShowString(4U, 7U, "Step4:Temp Check ", OLED_FONT_6X8);
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
 * @brief  閺嶈宓佽ぐ鎾冲閻樿埖鈧礁鎷板鑼垛偓妤佹閿涘矁顓哥粻妤勭箻鎼达妇娅ㄩ崚鍡樼槷閿涳拷0~100閿涳拷
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
 * @brief  閼汇儴绻樻惔锔炬閸掑棙鐦張澶婂綁閸栨牕鍨崚閿嬫煀 OLED 鏉╂稑瀹抽弶鈽呯礄闂冨弶顒涙０鎴犵畳閸忋劌鐫嗛崚閿嬫煀閿涳拷
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
 * @brief  Step1閿涙俺顕伴崣鏍︾瑏鐠猴拷 K_EN 鐠囧棗鍩嗛張鐔告箿閹垫挸绱戦柅姘朵壕閿涘本顥呭ù瀣╁▏閼宠棄鍟跨粣锟�
 *
 * 鐟欏嫬鍨敍娆縚EN = LOW 閳拷 闁岸浜鹃張鐔告箿閹垫挸绱戦敍鍫㈡埛閻㈤潧娅掗崥绋挎値閿涘TA HIGH閿涳拷
 *       K_EN = HIGH 閳拷 闁岸浜鹃張鐔告箿閸忔娊妫撮敍鍫㈡埛閻㈤潧娅掗弬顓炵磻閿涘TA LOW閿涳拷
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
            s_ctx.expected_open_ch = (uint8_t)(i + 1U); /* 閺嗗倽顔囬敍宀冨 >1 閸掓瑦妫ら弫锟� */
        }
    }

    if (low_cnt > 1U)
    {
        /* 婢舵俺鐭鹃崥灞炬娴ｈ儻鍏橀敍姘▏閼宠棄鍟跨粣锟� */
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
 * @brief  Step2閿涙岸鎷＄€碉拷 STA 娑撳孩婀￠張娑楃瑝缁楋妇娈戦柅姘朵壕閸欐垼鎹ｇ痪鐘绘晩閼村鍟�
 *
 * 缁涙牜鏆愰敍姘帥婢跺嫮鎮婇幍鈧張锟�"鎼存柨鍙ч梻锟�"闁岸浜鹃敍鍦lay_CloseChannel閿涘奔绱扮粩瀣祮闁插﹥鏂佹禍鎺楁敚閿涘绱�
 *       閸愬秴顦╅悶锟�"鎼存梹澧﹀鈧�"闁岸浜鹃敍鍫熸付婢舵矮绔存稉顏庣礉娴滄帡鏀ｅ銈嗘瀹歌尪袙闂勩倧绱氶妴锟�
 * 妤犲矁鐦夐敍姘辨暠 st_verify_step2() 閸︼拷 ST_STEP2_MS 鐡掑懏妞傞崥搴ょ殶閻€劊鈧拷
 */
static void st_exec_step2(void)
{
    if (!s_ctx.step_pass[0])
    {
        /* Step1 婢惰精瑙﹂敍鍫滃▏閼宠棄鍟跨粣渚婄礆閿涘本妫ゅ▔鏇炵暔閸忋劎绫傞柨锟� */
        printf("[SelfTest] Step2 SKIP: Step1 failed, no correction\r\n");
        return;
    }

    /* 缁楊兛绔存潪顕嗙窗閸忔娊妫撮幍鈧張澶婄安閸忔娊妫存担锟� STA 娴犲秳璐� HIGH 閻ㄥ嫰鈧岸浜� */
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

    /* 缁楊兛绨╂潪顕嗙窗閹垫挸绱戞惔鏃€澧﹀鈧担锟� STA 娴犲秳璐� LOW 閻ㄥ嫰鈧岸浜鹃敍鍫滅鞍闁夸礁鍑￠崷銊ь儑娑撯偓鏉烆喕鑵戠憴锝夋珟閿涳拷 */
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
 * @brief  Step2 缂佹挻娼弮鍫曠崣鐠囦焦澧嶉張澶愨偓姘朵壕 K_1_STA 閺勵垰鎯佹稉锟� K_EN 閺堢喐婀滄稉鈧懛锟�
 * @retval true = 閸忋劑鍎存稉鈧懛杈剧礄闁俺绻冮敍澶涚幢false = 鐎涙ê婀稉宥呭爱闁板稄绱欐径杈Е閿涳拷
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
 * @brief  Step3閿涙碍顥呴弻銉ょ瑏鐠猴拷 SW_STA閿涘牊甯寸憴锕€娅掗敍澶嬫Ц閸氾缚绗� K_EN 閺堢喐婀滄稉鈧懛锟�
 * @note   閹恒儴袝閸ｃ劍妫ゅ▔鏇⑩偓姘崇箖鏉烆垯娆㈢痪鐘绘晩閿涘奔绗夐崠褰掑帳閻╁瓨甯撮弽鍥唶婢惰精瑙�
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
 * @brief  Step4閿涙俺袝閸欐垳绔村▎鈩冧刊鎼达附娲块弬甯礉濡偓閺屻儰绗佺捄锟� NTC 閺勵垰鎯佹潻鍥ㄤ刊閿涘牃澧�60鎺矯閿涳拷
 */
static void st_exec_step4(void)
{
    Temperature_Update(); /* 閸掗攱鏌婂〒鈺佸閺佺増宓侀敍鍫ｅ殰濡偓閺堢喖妫� 1s 娴犺濮熼弳鍌氫粻閿涘矂娓堕幍瀣З鐟欙箑褰傞敍锟� */

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

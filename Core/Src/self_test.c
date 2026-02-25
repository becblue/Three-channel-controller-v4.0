/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : self_test.c
  * @brief          : 缂侇垵宕电划娲嚊椤忓拋姊炬俊顖椻偓铏仴閻庡湱鍋熼獮锟�
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-12
  ******************************************************************************
  * @attention
  *
  * 闁煎浜濋ˉ鍛规担琛℃煠闁哄啳娉涚花顓㈡晬閸繂褰欑紒鎾呮嫹 5.5 缂佸甯槐姘舵晬閿燂拷
  *   [0ms]     LOGO 闁哄嫬澧介妵锟� 2000ms
  *   [2000ms]  Step1 闁哄牏鍠愬﹢婊堟偐閼哥鍋撴担鐣屾闁告帪鎷� 500ms闁挎稑鐗愮换妯绘償閿燂拷 0%闁愁偓鎷�25%闁挎冻鎷�
  *   [2500ms]  Step2 缂備綀鍛毄闁革絻鍔庣猾鍌炴煥閿燂拷 800ms闁挎稑鐗愮换妯绘償閿燂拷 25%闁愁偓鎷�50%闁挎稑鑻幆锟� 500ms 闁兼潙顦崯璺ㄧ驳婢跺﹦绐￠柨娑虫嫹
  *   [3300ms]  Step3 闁规亽鍎磋闁革絻鍔嶉ˉ鍛村蓟閿燂拷 500ms闁挎稑鐗愮换妯绘償閿燂拷 50%闁愁偓鎷�75%闁挎冻鎷�
  *   [3800ms]  Step4 婵炴挴鏅涚€瑰啿螞閳ь剙霉閿燂拷 700ms闁挎稑鐗愮换妯绘償閿燂拷 75%闁愁偓鎷�100%闁挎冻鎷�
  *   [4500ms]  缂備焦鎸婚悘澶愬及閸撗佷粵 PASS/FAIL 1500ms闁挎稑鐬肩划銊╁级閿燂拷
  *
  * 闁稿繑濞婇弫顓犳媼閹规劦鍚€闁告ḿ鍠庨崹顖炴晬閿燂拷
  *   1. SelfTest_Update() 濞寸姴鎳忕敮瑙勬交濞戞粌娈版俊顐熷亾闁绘ǹ鍩栭埀顑跨劍濠р偓闁挎稑濂旂粭澶屾嫬閸愵亝鏆� Relay_Update()闁靛棴鎷�
  *      Relay_Update() 闁汇垹褰夌€靛苯顕ラ鍡楃畾 20ms 濞寸姾顕ф慨鐔煎箰娴ｈ櫣鏁惧鐟板船婵晠鏁嶅畝鈧垾妯荤┍閿燂拷 Step2 缂佸墽濞€閺佸﹪鎳樻径濠傛毐婵繐绲介悥鍫曞箥瑜戦、鎴﹀Υ閿燂拷
  *   2. Step2 闁革负鍔忕换姗€宕楅妷锔筋槯缂佹柨顑呭畵鍡涘矗閹达絾宕崇紒鍓у█閺佸﹪鎳樻径濠傛毐闁挎稑鑻﹢锟� 800ms 閻℃帒鎳忓鍌炲触鎼淬劎宕ｉ悹鍥锋嫹 STA 缂備焦鎸婚悘澶愬Υ閿燂拷
  *   3. 婵絽绻戦鐐参涢埀顒€霉鐎ｎ亜袟濞达絾绮撻埀顒佷亢缁伙拷 step_action_done 闁哄秴娲ょ换鏃€绌卞┑濠勬闁告瑯浜濇晶鐣屾偘鐏炶偐顏辨繛鍠℃壋鍋撻敓锟�
  *   4. OLED 閺夆晜绋戠€规娊寮堕垾鑼煂闁革负鍔庡▍銊╁礆閸℃ḿ妲烽柛娆惷€垫煡寮捄鍝勭厱闁哄倸搴滅槐婵嬫焼閸喖甯抽梻鍌や簽閸庡﹪濡撮敓锟�
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
#include <stdint.h>

/* Private defines -----------------------------------------------------------*/

/* 闁告艾瀚板Ο浣糕枔閸偄鐦紓渚囧幗濡炲倿姊绘潏鍓хms闁挎冻鎷� */
#define ST_LOGO_MS          TIME_LOGO_DISPLAY   ///< LOGO 闁哄嫬澧介妵姘跺籍閸洘姣愰柨娑虫嫹2000ms
#define ST_STEP1_MS         500U                ///< Step1 闁归晲鑳堕悽濠氬籍閸洘姣�
#define ST_STEP2_MS         800U                ///< Step2 闁归晲鑳堕悽濠氬籍閸洘姣愰柨娑虫嫹500ms闁兼潙顦崯锟� + 300ms缂佸鍟块悾楣冩晬閿燂拷
#define ST_STEP3_MS         500U                ///< Step3 闁归晲鑳堕悽濠氬籍閸洘姣�
#define ST_STEP4_MS         700U                ///< Step4 闁归晲鑳堕悽濠氬籍閸洘姣�
#define ST_RESULT_MS        1500U               ///< PASS/FAIL 缂備焦鎸婚悘澶愬磻濠婂懏娈岄柡鍐ㄧ埣閺嗭拷

/* 閺夆晜绋戠€规娊寮堕垾铏€楁慨婵勫劦椤庡啰鎸у畡閭︽綏闁糕晛鎼崳顖炴晬閸繂绀嬪ù锝呯▌缁憋拷%闁挎稑鑻崣锟� 100%闁挎冻鎷� */
#define PROG_STEP1_BASE     0U
#define PROG_STEP2_BASE     25U
#define PROG_STEP3_BASE     50U
#define PROG_STEP4_BASE     75U

/* Private types -------------------------------------------------------------*/

/**
 * @brief 闁煎浜濋ˉ鍛▔婵犱胶鐟撻柡鍌氭祫缁辨瑩寮甸埀顒備焊韫囨挸鏁堕梺顔哄妿婵悂骞€娓氬﹦绀�
 */
typedef struct
{
    SelfTest_State_e state;             ///< 鐟滅増鎸告晶鐘绘嚊椤忓拋姊鹃柣妯垮煐閳ь剨鎷�
    uint32_t         step_start_ms;     ///< 鐟滅増鎸告晶鐘绘偐閼哥鍋撴担鐣岀闁稿繈鍎插鍌炴儍閸曨剚顦ч梻鍌氱摠閸╋拷
    bool             step_action_done;  ///< 鐟滅増鎸告晶鐘差潰閵夆晩鈧啯绋夐弰蹇ｆ矗闁告柣鍔嬬紞鏃堝及椤栨碍鍎婄€圭ǹ寮舵晶鐣屾偘閿燂拷
    bool             step_pass[4];      ///< Step1~4 闁告艾瀚鐐电磼閹惧浜柨娑樼墔缁楀懘寮介敓锟� 0~3闁挎冻鎷�
    uint8_t          expected_open_ch;  ///< Step1 閻犲洤妫楅崺鍡涘礄閾忚鐣遍柡鍫㈠枑濠€婊堝箥閹惧磭纾婚梺顐ｅ哺娴滈箖鏁嶉敓锟�0=闁哄啰濯寸槐锟�1~3闁挎冻鎷�
    uint8_t          last_progress;     ///< 濞戞挸锕ラ锟� OLED 闁哄嫬澧介妵姘交濞戞ê顔婇柨娑樼墦濡插鏌屽鍜佹Щ闁告帡鏀遍弻濠囨晬閿燂拷0xFF=鐎殿喖鎼崺妤呭礆闁垮鐓€闁挎冻鎷�
    bool             initialized;       ///< 婵☆垪鈧櫕鍋ラ柡鍕靛灠閹礁顔忛幓鎺撳剻闁告棑鎷�
    bool             passed;            ///< 闁哄牃鍋撶紓浣哥墢缁劑寮搁敓锟�
} SelfTest_Ctx_t;

/* Private variables ---------------------------------------------------------*/

static SelfTest_Ctx_t s_ctx;

/**
 * @brief 濞戞挸顦甸埀顒佸哺娴滄儳顕ｉ弴鈥冲闁哄被鍎叉竟妯兼偘椤帞绀勯柟绋款樀閳ь剚宀告禍鍓ф閵忕姷绌� 0~2闁挎冻鎷�
 */
static const struct
{
    GPIO_TypeDef *en_port;        ///< K_EN 缂佹棏鍨拌ぐ锟�
    uint16_t      en_pin;         ///< K_EN 鐎殿喗娲濋崜锟�
    GPIO_TypeDef *k1_sta_port;    ///< K_1_STA 缂佹棏鍨拌ぐ锟�
    uint16_t      k1_sta_pin;     ///< K_1_STA 鐎殿喗娲濋崜锟�
    GPIO_TypeDef *sw_sta_port;    ///< SW_STA 缂佹棏鍨拌ぐ锟�
    uint16_t      sw_sta_pin;     ///< SW_STA 鐎殿喗娲濋崜锟�
    Channel_e     channel;        ///< 闂侇偅宀告禍楣冨几濮橆偄顩柛濠忔嫹
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
 * @brief  闁告凹鍨版慨鈺冨寲閼姐倗鍩犻柤濂変簼椤ワ拷
 */
void SelfTest_Start(void)
{
    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.last_progress = 0xFFU; /* 鐎殿喖鎼崺妤冪箔椤戣法顏辨繛鍠°倗绠婚幖杈鹃檮濞碱垶宕氶柨瀣厐 */
    s_ctx.initialized   = true;
    printf("[SelfTest] Starting self-test sequence...\r\n");
    st_enter_state(SELF_TEST_LOGO);
}

/**
 * @brief  闁哄被鍎撮妤呮嚊椤忓拋姊鹃柡鍕靛灠閹焦绂掑鍛含閺夆晜鍔橀、锟�
 */
uint8_t SelfTest_IsRunning(void)
{
    if (!s_ctx.initialized)
    {
        return 0U;
    }
    /* IDLE / PASS闁挎稑鐗忕划銊╁级閻旈攱鍊甸柨娑虫嫹/ FAIL闁挎稑鐗忕划銊╁级閻旈攱鍊甸柨娑橆槸濞煎海鎲撮崱鏇＄濞戞挸绉村﹢顏呮交閹邦垼鏀� */
    return (s_ctx.state != SELF_TEST_IDLE) ? 1U : 0U;
}

/**
 * @brief  闁哄被鍎撮妤呮嚊椤忓拋姊鹃柡鍕靛灠閹線鏌呭宕囩畺
 */
uint8_t SelfTest_IsPassed(void)
{
    return s_ctx.passed ? 1U : 0U;
}

/**
 * @brief  闁煎浜濋ˉ鍛存偐閼哥鍋撴担瑙勭皻闁哄洤鐡ㄩ弻濠囨晬閸儲濮滈梻鍐嚙椤綁鏁嶇仦鍓фЖ 20ms 閻犲鍟伴弫銈夋晬閿燂拷
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
                /* 閺夆晜绋戦崣锟� Step2 缂佹柨顑呭畵鍡涘矗閹达絾宕崇紒鍓у█閺佸﹪鎳樻径濠傛毐闁挎稑鐗忛弫杈ㄧ▔鐠囧弶鍎曢柣婊愭嫹 Relay_Update 闁圭瑳鍡╂斀闁挎冻鎷� */
                s_ctx.step_action_done = true;
                st_exec_step2();
            }
            if (elapsed >= ST_STEP2_MS)
            {
                /* 闁兼潙顦崯鎸庢償閺傚灝鍤掗悗鐟版湰閸ㄦ岸鏁嶉敓锟�500ms闁挎稑顧€缁辨繈宕樺鍥╂惣 300ms 闁告艾閰ｉ悰娆戞嫚閿燂拷 STA */
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
                /* OLED 闁哄嫬澧介妵锟� PASS */
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
                /* OLED 闁哄嫬澧介妵锟� FAIL */
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
 * @brief  闁瑰灚鎸稿畵鍐嚊椤忓拋姊鹃柛姘椤掔偞顨ラ妶鍥╂尝闁哄绮ｇ槐娆戞嫬閸愵厾妲搁柣銏╃厜缁憋拷
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
 * @brief  闁告帒娲﹀畷鏌ュ礆閻楀牊鐓€闁绘ǹ鍩栭埀顑跨筏缁辨繈鏌屽鍥╂瀭閻犱讲鍓濆鍌炲闯閵娿儲瀚查柛鏂诲妺缂嶆棃寮介崶褏绠堕柨娑樿嫰閼荤喖寮寸€涙ɑ鐓€ OLED 闁伙絽鐭傚ḿ锟�
 */
static void st_enter_state(SelfTest_State_e new_state)
{
    s_ctx.state            = new_state;
    s_ctx.step_start_ms    = HAL_GetTick();
    s_ctx.step_action_done = false;

    switch (new_state)
    {
        case SELF_TEST_LOGO:
            /* OLED_ShowLogo() 闁告劕鎳橀崕鏉戭啅閹绘帒鐦堕柛姘炬嫹 OLED_Clear() 闁告粣鎷� OLED_Refresh() */
            /* 显示 LOGO 同时发起三路强制关闭脉冲：
             * 磁保持继电器上电状态未知，需在 Step1 前确保全部断开。
             * OFF 脉冲 500ms，LOGO 停留 2000ms，关闭在 LOGO 期间完成。
             * Step2 纠错逻辑保留，两者互补。 */
            OLED_ShowLogo();
            Relay_ForceCloseAll();
            printf("[SelfTest] Showing LOGO (%ums), force-closing all channels...\r\n",
                   (unsigned int)ST_LOGO_MS);
            break;

        case SELF_TEST_STEP1_STATE_IDENTIFY:
            /*
             * 闁伙絽鐭傚ḿ锟�2閻㈩垰鍟惇顒勬晬閿燂拷
             *   row0 (y=0~7) : 闁哄秴娲。锟� "Self Testing..."
             *   row1~4(y=8~39): minyerLOGO闁挎稑鐗嗛惇铏▔椤撱劎绀�
             *   row5~6(y=40~55): 閺夆晜绋戠€规娊寮堕敓锟� + 闁谎勫劤閸ㄥ骸袙閿燂拷
             *   row7 (y=56~63): 婵縿鍎甸鍐触閿燂拷
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
            /* 闁告瑯浜滈崺娑㈠棘閿燂拷 row7闁挎稑鐗婇鐐搭殽閵堝懏鍊抽柨娑橆槸閹风増娼诲☉妯侯唺闁告牜灏ㄧ槐婵嬪冀閸ヮ剦鏆☉鎿勬嫹 LOGO 濞ｅ洦绻勯弳鈧� */
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
 * @brief  闁哄秷顫夊畵浣姐亹閹惧啿顤呴柣妯垮煐閳ь兛绀侀幏鏉款啅閼煎灈鍋撳Δ浣诡槯闁挎稑鐭侀鍝ョ不濡ゅ嫮绠婚幖杈惧濞呫劑宕氶崱妯兼Х闁挎冻鎷�0~100闁挎冻鎷�
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
 * @brief  闁兼眹鍎寸换妯绘償閿旂偓顏ラ柛鎺戞閻︻噣寮垫径濠傜秮闁告牗鐗曢崹顖炲礆闁垮鐓€ OLED 閺夆晜绋戠€规娊寮堕埥鍛闂傚啫寮堕娑欙紣閹寸姷鐣抽柛蹇嬪妼閻棝宕氶柨瀣厐闁挎冻鎷�
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
 * @brief  Step1闁挎稒淇洪浼村矗閺嶏妇鐟忛悹鐚存嫹 K_EN 閻犲洤妫楅崺鍡涘嫉閻斿憡绠块柟鍨尭缁辨垿鏌呭鏈靛闁挎稑鏈ˉ鍛圭€ｂ晛鈻忛柤瀹犳閸熻法绮ｉ敓锟�
 *
 * 閻熸瑥瀚崹顖炴晬濞嗙笟EN = LOW 闁愁偓鎷� 闂侇偅宀告禍楣冨嫉閻斿憡绠块柟鍨尭缁辨垿鏁嶉崼銏″煕闁汇垽娼у▍鎺楀触缁嬫寧鍊ら柨娑橆劔TA HIGH闁挎冻鎷�
 *       K_EN = HIGH 闁愁偓鎷� 闂侇偅宀告禍楣冨嫉閻斿憡绠块柛蹇斿▕濡挳鏁嶉崼銏″煕闁汇垽娼у▍鎺楀棘椤撶偟纾婚柨娑橆劔TA LOW闁挎冻鎷�
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
            s_ctx.expected_open_ch = (uint8_t)(i + 1U); /* 闁哄棗鍊介鍥晬瀹€鍐仧 >1 闁告帗鐟﹀Λ銈夊极閿燂拷 */
        }
    }

    if (low_cnt > 1U)
    {
        /* 濠㈣埖淇洪惌楣冨触鐏炵偓顦уù锝堝劵閸忔﹢鏁嶅顐⑩枏闁煎疇妫勯崯璺ㄧ玻閿燂拷 */
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
 * @brief  Step2闁挎稒宀搁幏锛勨偓纰夋嫹 STA 濞戞挸瀛╁﹢锟犲嫉濞戞鐟濈紒妤嬪濞堟垿鏌呭鏈靛闁告瑦鍨奸幑锝囩棯閻樼粯鏅╅柤鏉戭槸閸燂拷
 *
 * 缂佹稒鐗滈弳鎰版晬濮橆剙甯ュ璺哄閹﹪骞嶉埀顒勫嫉閿燂拷"閹煎瓨鏌ㄩ崣褔姊婚敓锟�"闂侇偅宀告禍楣冩晬閸︻湯lay_CloseChannel闁挎稑濂旂槐鎵博鐎ｎ亜绁梺鎻掞攻閺備焦绂嶉幒妤佹暁闁挎稑顧€缁憋拷
 *       闁告劕绉撮ˇ鈺呮偠閿燂拷"閹煎瓨姊规晶锕€顕ｉ埀锟�"闂侇偅宀告禍楣冩晬閸喐浠樺鑸电煯缁斿瓨绋夐搴ｇ濞存粍甯￠弨锝咁潰閵堝棙顦х€规瓕灏闂傚嫨鍊х槐姘跺Υ閿燂拷
 * 濡ょ姴鐭侀惁澶愭晬濮樿鲸鏆� st_verify_step2() 闁革讣鎷� ST_STEP2_MS 閻℃帒鎳忓鍌炲触鎼淬倗娈堕柣鈧妸閳ь剨鎷�
 */
static void st_exec_step2(void)
{
    if (!s_ctx.step_pass[0])
    {
        /* Step1 濠㈡儼绮剧憴锕傛晬閸粌鈻忛柤瀹犳閸熻法绮ｆ笟濠勭闁挎稑鏈Λ銈呪枖閺囩偟鏆旈柛蹇嬪妿缁倿鏌ㄩ敓锟� */
        printf("[SelfTest] Step2 SKIP: Step1 failed, no correction\r\n");
        return;
    }

    /* 缂佹鍏涚粩瀛樻姜椤曞棛绐楅柛蹇斿▕濡挳骞嶉埀顒勫嫉婢跺﹦瀹夐柛蹇斿▕濡瓨鎷呴敓锟� STA 濞寸姴绉崇拹锟� HIGH 闁汇劌瀚伴埀顒佸哺娴滐拷 */
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

    /* 缂佹鍏涚花鈺傛姜椤曞棛绐楅柟鍨尭缁辨垶鎯旈弮鈧晶锕€顕ｉ埀顒佹媴閿燂拷 STA 濞寸姴绉崇拹锟� LOW 闁汇劌瀚伴埀顒佸哺娴滈箖鏁嶉崼婊呴瀺闂佸じ绀侀崙锟犲捶閵娧屽剳濞戞挴鍋撻弶鐑嗗枙閼垫垹鎲撮敐澶嬬彑闁挎冻鎷� */
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
 * @brief  Step2 缂備焦鎸诲ḿ顐﹀籍閸洜宕ｉ悹鍥︾劍婢у秹寮垫径鎰ㄥ亾濮樻湹澹� K_1_STA 闁哄嫷鍨伴幆浣圭▔閿燂拷 K_EN 闁哄牏鍠愬﹢婊勭▔閳ь剟鎳涢敓锟�
 * @retval true = 闁稿繈鍔戦崕瀛樼▔閳ь剟鎳涙潏鍓х闂侇偅淇虹换鍐晬婢舵稓骞alse = 閻庢稒锚濠€顏呯▔瀹ュ懎鐖遍梺鏉跨▌缁辨瑦寰勬潏顐バ曢柨娑虫嫹
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
 * @brief  Step3闁挎稒纰嶉ˉ鍛村蓟閵夈倗鐟忛悹鐚存嫹 SW_STA闁挎稑鐗婄敮瀵告喆閿曗偓濞呮帡鏁嶆径瀣﹂柛姘剧細缁楋拷 K_EN 闁哄牏鍠愬﹢婊勭▔閳ь剟鎳涢敓锟�
 * @note   闁规亽鍎磋闁革絻鍔嶅Λ銈呪枖閺団懇鍋撳宕囩畺閺夌儐鍨▎銏㈢棯閻樼粯鏅╅柨娑樺缁楀宕犺ぐ鎺戝赋闁烩晛鐡ㄧ敮鎾冀閸ヮ亶鍞跺鎯扮簿鐟欙拷
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
 * @brief  Step4闁挎稒淇鸿闁告瑦鍨崇粩鏉戔枎閳╁啩鍒婇幖杈鹃檮濞插潡寮敮顔剧婵☆偀鍋撻柡灞诲劙缁椾胶鎹勯敓锟� NTC 闁哄嫷鍨伴幆浣规交閸ャ劋鍒婇柨娑樼墐婢э拷60閹虹煰闁挎冻鎷�
 */
static void st_exec_step4(void)
{
    Temperature_Update(); /* 闁告帡鏀遍弻濠傘€掗埡浣割唺闁轰胶澧楀畵渚€鏁嶉崼锝呮婵☆偀鍋撻柡鍫㈠枛濡拷 1s 濞寸姾顕ф慨鐔煎汲閸屾矮绮婚柨娑樼焸濞撳爼骞嶇€ｎ亜袟閻熸瑱绠戣ぐ鍌炴晬閿燂拷 */

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

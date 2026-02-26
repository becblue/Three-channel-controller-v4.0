/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : data_logger.c
  * @brief          : External Flash data logger implementation
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-25
  ******************************************************************************
  * @note
  *   Record write flow:
  *     1. Compute write address from metadata write_ptr
  *     2. Pre-erase ensures the target sector is blank before each write
  *     3. Call W25Q_PageProgram to write 16 bytes
  *     4. Update write_ptr and record_count
  *     5. Trigger pre-erase when current sector is >90% full
  *
  *   Pre-erase strategy (background, non-blocking):
  *     DataLogger_BackgroundTask (called every 100ms) performs sector erase
  *     and updates pre_erased_sector in metadata after completion.
  *
  *   Serial dump (KEY1 long-press 1s triggers):
  *     BackgroundTask outputs up to 8 records per call via huart3 (debug UART).
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "data_logger.h"
#include "w25q_flash.h"
#include "usart.h"
#include "main.h"
#include "oled_display.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define META_WRITE_ADDR         LOG_META_SECTOR_ADDR   /* Sector 0 base address */
#define DUMP_BATCH_SIZE         8U                     /* records output per BackgroundTask call */
#define KEY_STABLE_CONFIRM_CNT  50U                    /* 1000ms / 20ms = 50 counts (UART dump) */
#define KEY_5S_CNT              250U                   /* 5000ms / 20ms: enter OLED log */
#define KEY_10S_CNT             500U                   /* 10000ms / 20ms: exit OLED log */
#define UART_TX_TIMEOUT_MS      50U                    /* per-string UART TX timeout (ms) */
#define OLED_LOG_LINES          6U                     /* log lines per screen (row 0~5), row6=page, row7=hint */
#define OLED_LOG_LINE_CHARS     21U                    /* 6x8 font: 128/6 ~ 21 chars per line */

/* Private variables ---------------------------------------------------------*/
static bool     s_logger_ready        = false;  /* module ready flag */
static LogMeta_t s_meta;                        /* metadata cache (mirrors Sector 0) */
static bool     s_pre_erase_pending   = false;  /* background sector erase requested */
static uint32_t s_pre_erase_sector_abs = 0U;   /* target sector (relative to data area) */
static bool     s_dump_pending        = false;  /* serial dump in progress */
static uint32_t s_dump_read_ptr       = 0U;    /* dump read offset (bytes from DATA_START) */
static uint32_t s_dump_total          = 0U;    /* total records to dump this session */
static uint16_t s_key1_cnt            = 0U;   /* KEY1 press counter (20ms ticks) */
static uint16_t s_key2_cnt            = 0U;   /* KEY2 press counter (20ms ticks) */
static bool     s_format_pending      = false;  /* format request flag */
/* OLED log view: KEY1 5s enter, short press next/exit, KEY1 10s exit */
static bool     s_oled_log_active     = false;
static uint32_t s_oled_log_read_ptr   = 0U;    /* byte offset for current page start */
static uint32_t s_oled_log_total      = 0U;    /* total records to show */
static uint32_t s_oled_log_cur_page   = 0U;    /* 0-based page index */
static uint32_t s_oled_log_total_pages = 0U;
static bool     s_oled_log_need_draw  = false;
static uint16_t s_oled_log_key1_cnt   = 0U;    /* KEY1 hold count in OLED log mode (for 10s exit) */
static bool     s_key1_reached_1s     = false; /* true after 1s hold, to trigger UART dump on release if not 5s */

/* Private function prototypes -----------------------------------------------*/
static uint8_t  calc_crc8(const uint8_t *data, uint8_t len);
static bool     save_meta(void);
static bool     write_record(const LogRecord_t *rec);
static void     check_pre_erase(void);
static void     uart_print(const char *str);
static void     format_record_text(const LogRecord_t *rec, char *out, uint16_t out_len);
static void     format_record_short(const LogRecord_t *rec, char *out, uint16_t out_len);
static void     draw_oled_log_page(void);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Logger init: read/validate metadata, auto-init on first use, write BOOT record
  */
bool DataLogger_Init(void)
{
    uint8_t buf[sizeof(LogMeta_t)];

    /* Short delay so SPI/Flash is stable after power-on before first read */
    HAL_Delay(5U);

    if (W25Q_Read(META_WRITE_ADDR, buf, sizeof(LogMeta_t)) != W25Q_OK) {
        s_logger_ready = false;
        return false;
    }
    memcpy(&s_meta, buf, sizeof(LogMeta_t));

    /* Validate metadata: magic must match AND write_ptr must be within data area */
    if ((s_meta.magic != LOG_META_MAGIC) ||
        (s_meta.write_ptr >= (LOG_DATA_SECTOR_COUNT * W25Q_SECTOR_SIZE))) {
        /* First use, old/foreign data, or corrupted metadata: re-initialize */
        memset(&s_meta, 0, sizeof(LogMeta_t));
        s_meta.magic             = LOG_META_MAGIC;
        s_meta.write_ptr         = 0U;
        s_meta.record_count      = 0U;
        s_meta.wrap_count        = 0U;
        s_meta.pre_erased_sector = 0U;

        if (W25Q_SectorErase(META_WRITE_ADDR) != W25Q_OK) {
            s_logger_ready = false;
            return false;
        }
        if (!save_meta()) {
            s_logger_ready = false;
            return false;
        }
        /* Pre-erase first data sector */
        if (W25Q_SectorErase(LOG_DATA_START_ADDR) != W25Q_OK) {
            s_logger_ready = false;
            return false;
        }
    }

    s_logger_ready = true;
    DataLogger_WriteBoot();
    return true;
}

/**
  * @brief  Write a BOOT record (called automatically by DataLogger_Init)
  */
void DataLogger_WriteBoot(void)
{
    LogRecord_t rec;
    if (!s_logger_ready) { return; }
    memset(&rec, 0, sizeof(rec));
    rec.timestamp = HAL_GetTick() / 1000U;   /* seconds since power-on */
    rec.type      = (uint8_t)LOG_TYPE_BOOT;
    rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
    write_record(&rec);
}

/**
  * @brief  Write a channel action record (open or close)
  */
void DataLogger_WriteChannelAction(uint8_t ch, bool open)
{
    LogRecord_t rec;
    if (!s_logger_ready) { return; }
    memset(&rec, 0, sizeof(rec));
    rec.timestamp = HAL_GetTick() / 1000U;   /* seconds since power-on */
    rec.type      = open ? (uint8_t)LOG_TYPE_CH_OPEN : (uint8_t)LOG_TYPE_CH_CLOSE;
    rec.param1    = ch;
    rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
    write_record(&rec);
}

/**
  * @brief  Write an alarm event record (set or clear)
  */
void DataLogger_WriteAlarm(uint16_t alarm_type, bool is_set)
{
    LogRecord_t rec;
    if (!s_logger_ready) { return; }
    memset(&rec, 0, sizeof(rec));
    rec.timestamp = HAL_GetTick() / 1000U;   /* seconds since power-on */
    rec.type      = is_set ? (uint8_t)LOG_TYPE_ALARM_SET : (uint8_t)LOG_TYPE_ALARM_CLR;
    /* Store 16-bit alarm type across param1 (high byte) and param2 (low byte) */
    rec.param1    = (uint8_t)((alarm_type >> 8U) & 0xFFU);
    rec.param2    = (uint8_t)(alarm_type & 0xFFU);
    rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
    write_record(&rec);
}

/**
  * @brief  Trigger a serial dump of all log records
  */
void DataLogger_TriggerDump(void)
{
    char msg[64];
    if (!s_logger_ready || s_dump_pending) { return; }

    s_dump_read_ptr = 0U;
    s_dump_total    = (s_meta.record_count < LOG_MAX_RECORDS)
                      ? s_meta.record_count
                      : LOG_MAX_RECORDS;
    s_dump_pending  = true;

    uart_print("=== DATA LOGGER DUMP START ===\r\n");
    snprintf(msg, sizeof(msg), "Total: %lu records\r\n", (unsigned long)s_meta.record_count);
    uart_print(msg);
}

/**
  * @brief  Trigger logical format (resets Sector 0 metadata only)
  */
void DataLogger_TriggerFormat(void)
{
    if (!s_logger_ready) { return; }
    s_format_pending = true;
}

/**
  * @brief  Return whether OLED log view is active (main should call OledLogRefresh instead of main screen)
  */
bool DataLogger_IsOledLogActive(void)
{
    return s_oled_log_active;
}

/**
  * @brief  Refresh OLED log view: draw current page when need_draw (called from main every 500ms when active)
  */
void DataLogger_OledLogRefresh(void)
{
    if (!s_oled_log_active || !s_oled_log_need_draw) { return; }
    s_oled_log_need_draw = false;
    draw_oled_log_page();
}

/**
  * @brief  Return logger ready status
  */
bool DataLogger_IsReady(void)
{
    return s_logger_ready;
}

/**
  * @brief  Key scan (call every 20 ms from main loop)
  *         KEY1: 1s=UART dump (on release), 5s=enter OLED log, 10s(in log)=exit. KEY2: 1s=format.
  */
void DataLogger_KeyScan(void)
{
    /* KEY2 (PC14): format, unchanged */
    if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
        s_key2_cnt++;
        if (s_key2_cnt == KEY_STABLE_CONFIRM_CNT) {
            DataLogger_TriggerFormat();
        }
    } else {
        s_key2_cnt = 0U;
    }

    /* KEY1 (PC13) */
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {
        if (s_oled_log_active) {
            /* In OLED log mode: count for 10s exit */
            s_oled_log_key1_cnt++;
            if (s_oled_log_key1_cnt >= KEY_10S_CNT) {
                s_oled_log_active = false;
                s_oled_log_key1_cnt = 0U;
            }
        } else {
            s_key1_cnt++;
            if (s_key1_cnt >= KEY_STABLE_CONFIRM_CNT) {
                s_key1_reached_1s = true;
            }
            if (s_key1_cnt == KEY_5S_CNT) {
                /* Enter OLED log: do not trigger UART dump */
                s_oled_log_total = s_logger_ready
                    ? ((s_meta.record_count < LOG_MAX_RECORDS) ? s_meta.record_count : LOG_MAX_RECORDS)
                    : 0U;
                s_oled_log_total_pages = (s_oled_log_total > 0U)
                    ? ((s_oled_log_total + OLED_LOG_LINES - 1U) / OLED_LOG_LINES) : 1U;
                s_oled_log_active       = true;
                s_oled_log_read_ptr     = 0U;
                s_oled_log_cur_page     = 0U;
                s_oled_log_need_draw    = true;
                s_oled_log_key1_cnt     = 0U;
                s_key1_reached_1s       = false;
            }
        }
    } else {
        /* KEY1 released */
        if (s_oled_log_active) {
            if (s_oled_log_key1_cnt > 0U && s_oled_log_key1_cnt < KEY_10S_CNT) {
                /* Short press: next page or exit if last page */
                if (s_oled_log_cur_page + 1U >= s_oled_log_total_pages) {
                    s_oled_log_active = false;
                } else {
                    s_oled_log_cur_page++;
                    s_oled_log_read_ptr = s_oled_log_cur_page * OLED_LOG_LINES * LOG_RECORD_SIZE;
                    s_oled_log_need_draw = true;
                }
            }
            s_oled_log_key1_cnt = 0U;
        } else {
            if (s_key1_reached_1s && s_key1_cnt < KEY_5S_CNT) {
                DataLogger_TriggerDump();
            }
            s_key1_cnt        = 0U;
            s_key1_reached_1s = false;
        }
    }
}

/**
  * @brief  Background task (call every 100 ms from main loop)
  *         Handles: pre-erase, format, serial dump batching
  */
void DataLogger_BackgroundTask(void)
{
    if (!s_logger_ready) { return; }

    /* Format takes priority */
    if (s_format_pending) {
        s_format_pending = false;
        s_dump_pending   = false;

        uart_print("[LOGGER] Formatting Flash metadata...\r\n");

        if (W25Q_SectorErase(META_WRITE_ADDR) == W25Q_OK) {
            LogRecord_t rec;
            memset(&s_meta, 0, sizeof(LogMeta_t));
            s_meta.magic = LOG_META_MAGIC;
            save_meta();
            W25Q_SectorErase(LOG_DATA_START_ADDR);

            memset(&rec, 0, sizeof(rec));
            rec.timestamp = HAL_GetTick() / 1000U;   /* seconds since power-on */
            rec.type      = (uint8_t)LOG_TYPE_FORMAT;
            rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
            write_record(&rec);

            uart_print("[LOGGER] Format done.\r\n");
        } else {
            uart_print("[LOGGER] Format FAILED.\r\n");
        }
        return;
    }

    /* Background pre-erase (one sector per call, ~400 ms blocking) */
    if (s_pre_erase_pending) {
        uint32_t erase_addr;
        s_pre_erase_pending = false;
        erase_addr = LOG_DATA_START_ADDR + s_pre_erase_sector_abs * W25Q_SECTOR_SIZE;

        if (erase_addr < LOG_DATA_END_ADDR) {
            W25Q_SectorErase(erase_addr);
            s_meta.pre_erased_sector = (uint8_t)(s_pre_erase_sector_abs & 0xFFU);
            save_meta();
        }
        return;
    }

    /* Serial dump: output DUMP_BATCH_SIZE records per call */
    if (s_dump_pending) {
        uint32_t output_cnt = 0U;

        while ((s_dump_read_ptr < (s_dump_total * LOG_RECORD_SIZE)) &&
               (output_cnt < DUMP_BATCH_SIZE)) {
            uint32_t flash_addr = LOG_DATA_START_ADDR + s_dump_read_ptr;
            uint8_t  raw[LOG_RECORD_SIZE];
            char     line[80];

            if (W25Q_Read(flash_addr, raw, LOG_RECORD_SIZE) != W25Q_OK) {
                uart_print("[LOGGER] Read error.\r\n");
                s_dump_pending = false;
                return;
            }

            /* CRC check: skip corrupted records */
            if (calc_crc8(raw, (uint8_t)(LOG_RECORD_SIZE - 1U)) != raw[LOG_RECORD_SIZE - 1U]) {
                s_dump_read_ptr += LOG_RECORD_SIZE;
                output_cnt++;
                continue;
            }
            /* Type validity check: type=0x00 means blank Flash (0xFF erased then
             * programmed to 0x00 by another application), skip these garbage records */
            if (raw[4] == 0x00U) {
                s_dump_read_ptr += LOG_RECORD_SIZE;
                output_cnt++;
                continue;
            }

            {
                LogRecord_t rec;
                memcpy(&rec, raw, sizeof(rec));
                format_record_text(&rec, line, (uint16_t)sizeof(line));
                uart_print(line);
            }

            s_dump_read_ptr += LOG_RECORD_SIZE;
            output_cnt++;
        }

        if (s_dump_read_ptr >= (s_dump_total * LOG_RECORD_SIZE)) {
            uart_print("=== DUMP END ===\r\n");
            s_dump_pending = false;
        }
    }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CRC8 calculation (polynomial 0x07, init 0x00)
  */
static uint8_t calc_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00U;
    uint8_t i;
    uint8_t j;

    for (i = 0U; i < len; i++) {
        crc ^= data[i];
        for (j = 0U; j < 8U; j++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1U) ^ 0x07U);
            } else {
                crc = (uint8_t)(crc << 1U);
            }
        }
    }
    return crc;
}

/**
  * @brief  Flush metadata back to Sector 0 (erase + page program)
  */
static bool save_meta(void)
{
    uint8_t buf[sizeof(LogMeta_t)];

    if (W25Q_SectorErase(META_WRITE_ADDR) != W25Q_OK) {
        return false;
    }
    memcpy(buf, &s_meta, sizeof(LogMeta_t));
    return (W25Q_PageProgram(META_WRITE_ADDR, buf, (uint16_t)sizeof(LogMeta_t)) == W25Q_OK);
}

/**
  * @brief  Write one log record to Flash at current write_ptr
  */
static bool write_record(const LogRecord_t *rec)
{
    uint32_t write_addr;

    write_addr = LOG_DATA_START_ADDR + s_meta.write_ptr;

    /* Wrap around when reaching end of data area */
    if ((write_addr + LOG_RECORD_SIZE) > LOG_DATA_END_ADDR) {
        s_meta.write_ptr = 0U;
        s_meta.wrap_count++;
        write_addr = LOG_DATA_START_ADDR;
    }

    if (W25Q_PageProgram(write_addr, (const uint8_t *)rec, (uint16_t)LOG_RECORD_SIZE)
        != W25Q_OK) {
        return false;
    }

    s_meta.write_ptr    += LOG_RECORD_SIZE;
    s_meta.record_count++;

    check_pre_erase();

    /* Persist metadata after every write so reboot sees correct write_ptr/record_count
     * and does not overwrite from 0 or show wrong total (Sector 0 wear acceptable) */
    if (!save_meta()) {
        return false;
    }

    return true;
}

/**
  * @brief  Trigger pre-erase of the next sector when current is >90% full
  */
static void check_pre_erase(void)
{
    uint32_t cur_abs_addr    = LOG_DATA_START_ADDR + s_meta.write_ptr;
    uint32_t cur_sector_base = (cur_abs_addr / W25Q_SECTOR_SIZE) * W25Q_SECTOR_SIZE;
    uint32_t cur_sector_end  = cur_sector_base + W25Q_SECTOR_SIZE;
    uint32_t remaining       = cur_sector_end - cur_abs_addr;
    uint32_t threshold       = (W25Q_SECTOR_SIZE * (100U - LOG_PRE_ERASE_THRESHOLD)) / 100U;
    uint32_t next_sector_abs;
    uint32_t next_sector_rel;

    if (remaining <= threshold) {
        next_sector_abs = cur_sector_end;
        if (next_sector_abs >= LOG_DATA_END_ADDR) {
            next_sector_abs = LOG_DATA_START_ADDR;
        }
        next_sector_rel = (next_sector_abs - LOG_DATA_START_ADDR) / W25Q_SECTOR_SIZE;

        if (next_sector_rel != (uint32_t)s_meta.pre_erased_sector) {
            s_pre_erase_sector_abs = next_sector_rel;
            s_pre_erase_pending    = true;
        }
    }
}

/**
  * @brief  Send a string via UART3 (same as printf debug port, 115200 bps)
  */
static void uart_print(const char *str)
{
    uint16_t len = (uint16_t)strlen(str);
    if (len == 0U) { return; }
    HAL_UART_Transmit(&huart3, (uint8_t *)str, len, UART_TX_TIMEOUT_MS);
}

/**
  * @brief  Return single letter (A-O) for OLED short display
  */
static char alarm_type_letter(uint16_t t)
{
    switch (t) {
        case 0x0001U: return 'A';
        case 0x0002U: return 'B';
        case 0x0004U: return 'C';
        case 0x0008U: return 'D';
        case 0x0010U: return 'E';
        case 0x0020U: return 'F';
        case 0x0040U: return 'G';
        case 0x0080U: return 'H';
        case 0x0100U: return 'I';
        case 0x0200U: return 'J';
        case 0x0400U: return 'K';
        case 0x0800U: return 'L';
        case 0x1000U: return 'M';
        case 0x2000U: return 'N';
        case 0x4000U: return 'O';
        default:      return '?';
    }
}

/**
  * @brief  Return human-readable alarm name for a 16-bit ErrorType_e value
  */
static const char *alarm_type_name(uint16_t t)
{
    switch (t) {
        case 0x0001U: return "Enable conflict (A)";
        case 0x0002U: return "K1_1 feedback error (B)";
        case 0x0004U: return "K2_1 feedback error (C)";
        case 0x0008U: return "K3_1 feedback error (D)";
        case 0x0010U: return "K1_2 feedback error (E)";
        case 0x0020U: return "K2_2 feedback error (F)";
        case 0x0040U: return "K3_2 feedback error (G)";
        case 0x0080U: return "SW1 feedback error (H)";
        case 0x0100U: return "SW2 feedback error (I)";
        case 0x0200U: return "SW3 feedback error (J)";
        case 0x0400U: return "NTC1 overheat (K)";
        case 0x0800U: return "NTC2 overheat (L)";
        case 0x1000U: return "NTC3 overheat (M)";
        case 0x2000U: return "Self-test error (N)";
        case 0x4000U: return "DC power loss (O)";
        default:      return "Unknown alarm";
    }
}

/**
  * @brief  Format one log record into a human-readable line
  *         Timestamp in seconds; display e.g. "[ 5 s]" or "[ 2 min 3 s]" or "[ 1 h 2 min 3 s]"
  */
static void format_record_text(const LogRecord_t *rec, char *out, uint16_t out_len)
{
    uint32_t ts = rec->timestamp;  /* seconds since power-on */
    char event[56] = {0};

    switch ((LogType_e)rec->type) {
        case LOG_TYPE_BOOT:
            snprintf(event, sizeof(event), "BOOT");
            break;
        case LOG_TYPE_SELF_TEST_OK:
            snprintf(event, sizeof(event), "SELF-TEST PASS");
            break;
        case LOG_TYPE_SELF_TEST_NG:
            snprintf(event, sizeof(event), "SELF-TEST FAIL (code=0x%02X)", rec->param1);
            break;
        case LOG_TYPE_CH_OPEN:
            snprintf(event, sizeof(event), "CH%u OPEN", (unsigned)rec->param1);
            break;
        case LOG_TYPE_CH_CLOSE:
            snprintf(event, sizeof(event), "CH%u CLOSE", (unsigned)rec->param1);
            break;
        case LOG_TYPE_ALARM_SET: {
            uint16_t atype = (uint16_t)((uint16_t)rec->param1 << 8U) | (uint16_t)rec->param2;
            snprintf(event, sizeof(event), "ALARM SET  : %s", alarm_type_name(atype));
            break;
        }
        case LOG_TYPE_ALARM_CLR: {
            uint16_t atype = (uint16_t)((uint16_t)rec->param1 << 8U) | (uint16_t)rec->param2;
            snprintf(event, sizeof(event), "ALARM CLR  : %s", alarm_type_name(atype));
            break;
        }
        case LOG_TYPE_FORMAT:
            snprintf(event, sizeof(event), "FLASH FORMAT");
            break;
        default:
            snprintf(event, sizeof(event), "UNKNOWN(0x%02X) P1=0x%02X P2=0x%02X",
                     rec->type, rec->param1, rec->param2);
            break;
    }

    if (ts >= 3600U) {
        snprintf(out, out_len, "[%lu h %lu min %lu s] %s\r\n",
                 (unsigned long)(ts / 3600U),
                 (unsigned long)((ts % 3600U) / 60U),
                 (unsigned long)(ts % 60U), event);
    } else if (ts >= 60U) {
        snprintf(out, out_len, "[%lu min %lu s] %s\r\n",
                 (unsigned long)(ts / 60U), (unsigned long)(ts % 60U), event);
    } else {
        snprintf(out, out_len, "[%lu s] %s\r\n", (unsigned long)ts, event);
    }
}

/**
  * @brief  Short format for OLED (one line ~21 chars): "101s BOOT", "2m3s CH2 OP"
  */
static void format_record_short(const LogRecord_t *rec, char *out, uint16_t out_len)
{
    uint32_t ts = rec->timestamp;
    char ts_buf[12];
    char ev_buf[12];

    if (ts >= 3600U) {
        snprintf(ts_buf, sizeof(ts_buf), "%luh%lum", (unsigned long)(ts / 3600U), (unsigned long)((ts % 3600U) / 60U));
    } else if (ts >= 60U) {
        snprintf(ts_buf, sizeof(ts_buf), "%lum%lus", (unsigned long)(ts / 60U), (unsigned long)(ts % 60U));
    } else {
        snprintf(ts_buf, sizeof(ts_buf), "%lus", (unsigned long)ts);
    }

    switch ((LogType_e)rec->type) {
        case LOG_TYPE_BOOT:           snprintf(ev_buf, sizeof(ev_buf), "BOOT"); break;
        case LOG_TYPE_SELF_TEST_OK:   snprintf(ev_buf, sizeof(ev_buf), "OK"); break;
        case LOG_TYPE_SELF_TEST_NG:   snprintf(ev_buf, sizeof(ev_buf), "NG"); break;
        case LOG_TYPE_CH_OPEN:        snprintf(ev_buf, sizeof(ev_buf), "CH%u OP", (unsigned)rec->param1); break;
        case LOG_TYPE_CH_CLOSE:       snprintf(ev_buf, sizeof(ev_buf), "CH%u CL", (unsigned)rec->param1); break;
        case LOG_TYPE_ALARM_SET: {
            uint16_t at = (uint16_t)((uint16_t)rec->param1 << 8U) | (uint16_t)rec->param2;
            snprintf(ev_buf, sizeof(ev_buf), "ALM %c", alarm_type_letter(at));
            break;
        }
        case LOG_TYPE_ALARM_CLR: {
            uint16_t at = (uint16_t)((uint16_t)rec->param1 << 8U) | (uint16_t)rec->param2;
            snprintf(ev_buf, sizeof(ev_buf), "ALM %c CL", alarm_type_letter(at));
            break;
        }
        case LOG_TYPE_FORMAT:         snprintf(ev_buf, sizeof(ev_buf), "FMT"); break;
        default:                      snprintf(ev_buf, sizeof(ev_buf), "?"); break;
    }
    snprintf(out, out_len, "%-8s %-12s", ts_buf, ev_buf);
}

/**
  * @brief  Draw one page of log on OLED (rows 0~5 log lines, row6 page, row7 hint)
  */
static void draw_oled_log_page(void)
{
    uint8_t raw[LOG_RECORD_SIZE];
    char line_buf[OLED_LOG_LINE_CHARS + 2];
    uint32_t i;
    uint32_t total = s_oled_log_total;
    uint32_t total_pages = s_oled_log_total_pages;
    uint32_t read_ptr = s_oled_log_read_ptr;
    uint32_t cur_page = s_oled_log_cur_page;

    OLED_Clear();
    if (total == 0U) {
        OLED_ShowString(0, 0, "No records        ", OLED_FONT_6X8);
        OLED_ShowString(0, 2, "KEY1: exit        ", OLED_FONT_6X8);
        OLED_Refresh();
        return;
    }

    for (i = 0U; i < OLED_LOG_LINES; i++) {
        uint32_t rec_off = read_ptr + i * LOG_RECORD_SIZE;
        if (rec_off >= (total * LOG_RECORD_SIZE)) {
            OLED_ShowString(0, (uint8_t)i, "                    ", OLED_FONT_6X8);
            continue;
        }
        if (W25Q_Read(LOG_DATA_START_ADDR + rec_off, raw, LOG_RECORD_SIZE) != W25Q_OK) {
            OLED_ShowString(0, (uint8_t)i, "Read err           ", OLED_FONT_6X8);
            continue;
        }
        if (calc_crc8(raw, (uint8_t)(LOG_RECORD_SIZE - 1U)) != raw[LOG_RECORD_SIZE - 1U] || raw[4] == 0x00U) {
            OLED_ShowString(0, (uint8_t)i, "-                 ", OLED_FONT_6X8);
            continue;
        }
        {
            LogRecord_t rec;
            memcpy(&rec, raw, sizeof(rec));
            format_record_short(&rec, line_buf, sizeof(line_buf));
            line_buf[OLED_LOG_LINE_CHARS] = '\0';
            OLED_ShowString(0, (uint8_t)i, line_buf, OLED_FONT_6X8);
        }
    }

    snprintf(line_buf, sizeof(line_buf), "P %lu/%lu  KEY1:next", (unsigned long)(cur_page + 1U), (unsigned long)total_pages);
    OLED_ShowString(0, 6, line_buf, OLED_FONT_6X8);
    OLED_ShowString(0, 7, "KEY1 10s: exit     ", OLED_FONT_6X8);
    OLED_Refresh();
}

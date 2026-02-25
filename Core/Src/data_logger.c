/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : data_logger.c
  * @brief          : 外部 Flash 数据日志模块实现
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-25
  ******************************************************************************
  * @note
  *   日志记录写入流程：
  *     1. 计算写入地址（基于元数据 write_ptr）
  *     2. 检查当前地址是否在已擦除扇区内（预擦除保证）
  *     3. 调用 W25Q_PageProgram 写入 16 字节
  *     4. 更新 write_ptr 和 record_count
  *     5. 若写指针超过当前扇区 90%，触发预擦除标志
  *
  *   预擦除策略（非阻塞分片）：
  *     在 DataLogger_BackgroundTask（100ms 调用）中执行扇区擦除，
  *     擦除完成后更新 pre_erased_sector 并保存元数据。
  *
  *   串口日志输出（KEY1 长按 3s 触发）：
  *     在 DataLogger_BackgroundTask 中，每次输出最多 8 条记录，
  *     通过 huart1 以 ASCII 格式输出，避免单次阻塞过长。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "data_logger.h"
#include "w25q_flash.h"
#include "usart.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
/** @brief 元数据写入地址（Sector 0 起始）*/
#define META_WRITE_ADDR         LOG_META_SECTOR_ADDR

/** @brief 每次串口输出批次大小（条）*/
#define DUMP_BATCH_SIZE         8U

/** @brief KEY 防抖稳定确认次数（每 20ms 调用一次，50次 = 1s，150次=3s）*/
#define KEY_STABLE_CONFIRM_CNT  150U   ///< 3000ms / 20ms = 150 次

/** @brief 串口发送超时（ms）*/
#define UART_TX_TIMEOUT_MS      50U

/* Private variables ---------------------------------------------------------*/

/** @brief 模块就绪标志 */
static bool  s_logger_ready = false;

/** @brief 运行时元数据缓存（从 Flash 读入，写操作后同步刷回）*/
static LogMeta_t s_meta;

/** @brief 预擦除待处理标志 */
static bool s_pre_erase_pending = false;

/** @brief 待预擦除的扇区绝对号（相对 W25Q 全 Flash）*/
static uint32_t s_pre_erase_sector_abs = 0U;

/** @brief 串口日志转储状态 */
static bool     s_dump_pending    = false;
static uint32_t s_dump_read_ptr   = 0U;   ///< 当前转储读位置（字节偏移，相对 LOG_DATA_START_ADDR）
static uint32_t s_dump_total      = 0U;   ///< 本次转储需输出记录总数

/** @brief KEY1/KEY2 长按计数器（20ms 周期调用递增）*/
static uint16_t s_key1_cnt = 0U;
static uint16_t s_key2_cnt = 0U;

/** @brief 格式化请求标志 */
static bool s_format_pending = false;

/* Private function prototypes -----------------------------------------------*/
static uint8_t  calc_crc8(const uint8_t *data, uint8_t len);
static bool     save_meta(void);
static bool     write_record(const LogRecord_t *rec);
static uint32_t get_sector_addr_from_ptr(uint32_t write_ptr_offset);
static void     check_pre_erase(void);
static void     uart_print(const char *str);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  日志模块初始化
  */
bool DataLogger_Init(void)
{
    /* 尝试从 Sector 0 读取元数据 */
    uint8_t buf[sizeof(LogMeta_t)];
    if (W25Q_Read(META_WRITE_ADDR, buf, sizeof(LogMeta_t)) != W25Q_OK) {
        s_logger_ready = false;
        return false;
    }
    memcpy(&s_meta, buf, sizeof(LogMeta_t));

    /* 验证魔数 */
    if (s_meta.magic != LOG_META_MAGIC) {
        /* 首次使用或已格式化：初始化元数据 */
        memset(&s_meta, 0, sizeof(LogMeta_t));
        s_meta.magic              = LOG_META_MAGIC;
        s_meta.write_ptr          = 0U;
        s_meta.record_count       = 0U;
        s_meta.wrap_count         = 0U;
        s_meta.pre_erased_sector  = 0U;

        /* 擦除 Sector 0，写入初始元数据 */
        if (W25Q_SectorErase(META_WRITE_ADDR) != W25Q_OK) {
            s_logger_ready = false;
            return false;
        }
        if (!save_meta()) {
            s_logger_ready = false;
            return false;
        }
        /* 预擦除第一个数据扇区 */
        if (W25Q_SectorErase(LOG_DATA_START_ADDR) != W25Q_OK) {
            s_logger_ready = false;
            return false;
        }
    }

    s_logger_ready = true;

    /* 写入 BOOT 记录 */
    DataLogger_WriteBoot();

    return true;
}

/**
  * @brief  写入系统启动记录
  */
void DataLogger_WriteBoot(void)
{
    if (!s_logger_ready) { return; }

    LogRecord_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.timestamp = HAL_GetTick();
    rec.type      = (uint8_t)LOG_TYPE_BOOT;
    rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
    write_record(&rec);
}

/**
  * @brief  写入通道动作记录
  */
void DataLogger_WriteChannelAction(uint8_t ch, bool open)
{
    if (!s_logger_ready) { return; }

    LogRecord_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.timestamp = HAL_GetTick();
    rec.type      = open ? (uint8_t)LOG_TYPE_CH_OPEN : (uint8_t)LOG_TYPE_CH_CLOSE;
    rec.param1    = ch;
    rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
    write_record(&rec);
}

/**
  * @brief  写入报警事件记录
  */
void DataLogger_WriteAlarm(uint8_t alarm_type, bool is_set)
{
    if (!s_logger_ready) { return; }

    LogRecord_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.timestamp = HAL_GetTick();
    rec.type      = is_set ? (uint8_t)LOG_TYPE_ALARM_SET : (uint8_t)LOG_TYPE_ALARM_CLR;
    rec.param1    = alarm_type;
    rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
    write_record(&rec);
}

/**
  * @brief  触发串口日志转储
  */
void DataLogger_TriggerDump(void)
{
    if (!s_logger_ready || s_dump_pending) { return; }

    /* 从头开始转储所有有效记录 */
    s_dump_read_ptr = 0U;
    s_dump_total    = (s_meta.record_count < LOG_MAX_RECORDS)
                      ? s_meta.record_count
                      : LOG_MAX_RECORDS;
    s_dump_pending  = true;

    uart_print("=== DATA LOGGER DUMP START ===\r\n");
    char buf[64];
    snprintf(buf, sizeof(buf),
             "Total: %lu records\r\n", (unsigned long)s_meta.record_count);
    uart_print(buf);
}

/**
  * @brief  触发逻辑格式化
  */
void DataLogger_TriggerFormat(void)
{
    if (!s_logger_ready) { return; }
    s_format_pending = true;
}

/**
  * @brief  获取模块就绪状态
  */
bool DataLogger_IsReady(void)
{
    return s_logger_ready;
}

/**
  * @brief  按键轮询扫描（20ms 周期调用）
  * @note   KEY 为低电平有效（上拉），按下时 GPIO 读为 0
  */
void DataLogger_KeyScan(void)
{
    /* KEY1（PC13）：按下状态为低电平 */
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {
        s_key1_cnt++;
        if (s_key1_cnt == KEY_STABLE_CONFIRM_CNT) {
            /* 长按 3s 确认，触发转储 */
            DataLogger_TriggerDump();
        }
    } else {
        s_key1_cnt = 0U;
    }

    /* KEY2（PC14）：按下状态为低电平 */
    if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
        s_key2_cnt++;
        if (s_key2_cnt == KEY_STABLE_CONFIRM_CNT) {
            /* 长按 3s 确认，触发格式化 */
            DataLogger_TriggerFormat();
        }
    } else {
        s_key2_cnt = 0U;
    }
}

/**
  * @brief  后台任务（100ms 周期调用）
  */
void DataLogger_BackgroundTask(void)
{
    if (!s_logger_ready) { return; }

    /* 优先处理格式化请求 */
    if (s_format_pending) {
        s_format_pending  = false;
        s_dump_pending    = false;

        uart_print("[LOGGER] Formatting Flash metadata...\r\n");

        /* 擦除 Sector 0，重置元数据 */
        if (W25Q_SectorErase(META_WRITE_ADDR) == W25Q_OK) {
            memset(&s_meta, 0, sizeof(LogMeta_t));
            s_meta.magic = LOG_META_MAGIC;
            save_meta();

            /* 预擦除第一个数据扇区 */
            W25Q_SectorErase(LOG_DATA_START_ADDR);

            /* 写入格式化记录 */
            LogRecord_t rec;
            memset(&rec, 0, sizeof(rec));
            rec.timestamp = HAL_GetTick();
            rec.type      = (uint8_t)LOG_TYPE_FORMAT;
            rec.crc8      = calc_crc8((uint8_t *)&rec, (uint8_t)(sizeof(rec) - 1U));
            write_record(&rec);

            uart_print("[LOGGER] Format done.\r\n");
        } else {
            uart_print("[LOGGER] Format FAILED.\r\n");
        }
        return;
    }

    /* 预擦除处理（耗时约 400ms，每次只处理一个扇区）*/
    if (s_pre_erase_pending) {
        s_pre_erase_pending = false;
        uint32_t erase_addr = LOG_DATA_START_ADDR
                            + (uint32_t)s_pre_erase_sector_abs * W25Q_SECTOR_SIZE;

        /* 地址越界保护 */
        if (erase_addr < LOG_DATA_END_ADDR) {
            W25Q_SectorErase(erase_addr);
            /* 更新已预擦除扇区号并刷回元数据 */
            s_meta.pre_erased_sector = (uint8_t)(s_pre_erase_sector_abs & 0xFFU);
            save_meta();
        }
        return; /* 本次任务只做擦除，其余推迟 */
    }

    /* 串口转储处理（每次输出 DUMP_BATCH_SIZE 条）*/
    if (s_dump_pending) {
        uint32_t output_cnt = 0U;
        while ((s_dump_read_ptr < (s_dump_total * LOG_RECORD_SIZE)) &&
               (output_cnt < DUMP_BATCH_SIZE)) {

            uint32_t flash_addr = LOG_DATA_START_ADDR + s_dump_read_ptr;
            uint8_t raw[LOG_RECORD_SIZE];

            if (W25Q_Read(flash_addr, raw, LOG_RECORD_SIZE) != W25Q_OK) {
                uart_print("[LOGGER] Read error.\r\n");
                s_dump_pending = false;
                return;
            }

            /* CRC 校验 */
            uint8_t crc_calc = calc_crc8(raw, (uint8_t)(LOG_RECORD_SIZE - 1U));
            if (crc_calc != raw[LOG_RECORD_SIZE - 1U]) {
                /* 跳过损坏记录 */
                s_dump_read_ptr += LOG_RECORD_SIZE;
                output_cnt++;
                continue;
            }

            /* 解析并格式化输出 */
            LogRecord_t rec;
            memcpy(&rec, raw, sizeof(rec));

            char line[80];
            snprintf(line, sizeof(line),
                     "[%10lu] T:0x%02X P1:0x%02X P2:0x%02X EX:0x%08lX\r\n",
                     (unsigned long)rec.timestamp,
                     rec.type, rec.param1, rec.param2,
                     (unsigned long)rec.extra);
            uart_print(line);

            s_dump_read_ptr += LOG_RECORD_SIZE;
            output_cnt++;
        }

        /* 全部输出完成 */
        if (s_dump_read_ptr >= (s_dump_total * LOG_RECORD_SIZE)) {
            uart_print("=== DUMP END ===\r\n");
            s_dump_pending = false;
        }
    }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  计算 CRC8（多项式 0x07，初始值 0x00）
  */
static uint8_t calc_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00U;
    for (uint8_t i = 0U; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0U; j < 8U; j++) {
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
  * @brief  将元数据刷写回 Flash Sector 0
  * @note   调用前须确保 Sector 0 已擦除；此处采用页编程，不预先擦除
  *         （元数据仅占 ~24 字节，Sector 0 初始化时整扇区已擦除，
  *          后续 save_meta 直接页编程覆盖同一位置即可，
  *          因为 Flash 只能从 1→0 写，元数据更新需先擦扇区再写）
  * @retval true=成功，false=失败
  */
static bool save_meta(void)
{
    /* 每次更新元数据需要：擦除 Sector 0 → 写页 */
    if (W25Q_SectorErase(META_WRITE_ADDR) != W25Q_OK) {
        return false;
    }
    uint8_t buf[sizeof(LogMeta_t)];
    memcpy(buf, &s_meta, sizeof(LogMeta_t));
    return (W25Q_PageProgram(META_WRITE_ADDR, buf, (uint16_t)sizeof(LogMeta_t)) == W25Q_OK);
}

/**
  * @brief  将一条记录写入 Flash（含地址计算、溢出回绕、预擦除触发）
  */
static bool write_record(const LogRecord_t *rec)
{
    /* 计算本次写入的绝对 Flash 地址 */
    uint32_t write_addr = LOG_DATA_START_ADDR + s_meta.write_ptr;

    /* 若超出数据区末尾，回绕到起始位置（循环缓冲）*/
    if (write_addr + LOG_RECORD_SIZE > LOG_DATA_END_ADDR) {
        s_meta.write_ptr = 0U;
        s_meta.wrap_count++;
        write_addr = LOG_DATA_START_ADDR;
    }

    /* 执行页编程（16 字节）*/
    if (W25Q_PageProgram(write_addr, (const uint8_t *)rec, (uint16_t)LOG_RECORD_SIZE)
        != W25Q_OK) {
        return false;
    }

    /* 更新写指针和计数 */
    s_meta.write_ptr    += LOG_RECORD_SIZE;
    s_meta.record_count++;

    /* 检查是否需要触发预擦除 */
    check_pre_erase();

    /* 将元数据刷回 Flash（低频操作，每 16 条记录刷一次，减少 Sector 0 磨损）*/
    if ((s_meta.record_count % 16U) == 0U) {
        save_meta();
    }

    return true;
}

/**
  * @brief  获取 write_ptr 对应的扇区起始地址
  */
static uint32_t get_sector_addr_from_ptr(uint32_t write_ptr_offset)
{
    uint32_t abs_addr = LOG_DATA_START_ADDR + write_ptr_offset;
    return (abs_addr / W25Q_SECTOR_SIZE) * W25Q_SECTOR_SIZE;
}

/**
  * @brief  检查是否需要预擦除下一扇区（90% 阈值触发）
  */
static void check_pre_erase(void)
{
    /* 当前扇区内偏移 */
    uint32_t cur_sector_base = get_sector_addr_from_ptr(s_meta.write_ptr);
    uint32_t cur_sector_end  = cur_sector_base + W25Q_SECTOR_SIZE;
    uint32_t remaining       = cur_sector_end - (LOG_DATA_START_ADDR + s_meta.write_ptr);
    uint32_t threshold       = (W25Q_SECTOR_SIZE * (100U - LOG_PRE_ERASE_THRESHOLD)) / 100U;

    if (remaining <= threshold) {
        /* 计算下一扇区地址 */
        uint32_t next_sector_abs = LOG_DATA_START_ADDR + s_meta.write_ptr + remaining;
        if (next_sector_abs >= LOG_DATA_END_ADDR) {
            /* 回绕：下一扇区是数据区第一个 */
            next_sector_abs = LOG_DATA_START_ADDR;
        }

        /* 转换为相对扇区号 */
        uint32_t next_sector_rel = (next_sector_abs - LOG_DATA_START_ADDR) / W25Q_SECTOR_SIZE;

        /* 避免重复触发同一扇区 */
        if (next_sector_rel != (uint32_t)s_meta.pre_erased_sector) {
            s_pre_erase_sector_abs = next_sector_rel;
            s_pre_erase_pending    = true;
        }
    }
}

/**
  * @brief  串口输出字符串（使用 huart1，115200bps）
  */
static void uart_print(const char *str)
{
    uint16_t len = (uint16_t)strlen(str);
    if (len == 0U) { return; }
    HAL_UART_Transmit(&huart1, (uint8_t *)str, len, UART_TX_TIMEOUT_MS);
}

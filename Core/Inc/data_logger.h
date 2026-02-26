/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : data_logger.h
  * @brief          : External Flash data logger module header
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-25
  ******************************************************************************
  * @note
  *   Flash layout (W25Q128JVSIQ, 16 MB):
  *     Sector 0  (0x000000, 4 KB) : Metadata area (write_ptr, record_count, ...)
  *     Sector 1  (0x001000, 4 KB) : Reserved
  *     Sector 2..4095             : Log data area (~16 MB - 8 KB)
  *
 *   Record format (16 bytes fixed):
 *     Byte  0-3  : timestamp  (seconds since power-on, uint32_t, ~136 years to overflow)
  *     Byte  4    : type       (LOG_TYPE_xxx)
  *     Byte  5    : param1
  *     Byte  6    : param2
  *     Byte  7    : reserved   (0x00)
  *     Byte  8-11 : extra      (uint32_t, type-specific)
  *     Byte 12-14 : reserved   (0x00)
  *     Byte 15    : crc8       (CRC8 over bytes 0..14)
  *
  *   Circular buffer strategy:
  *     When write_ptr reaches 90% of a sector, the next sector is
  *     pre-erased asynchronously in BackgroundTask.
  *
  *   Key operations (hold 1 s):
  *     KEY1 (PC13) : dump all logs to UART3 (debug port, 115200 bps)
  *     KEY2 (PC14) : logical format (reset Sector 0 metadata only)
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __DATA_LOGGER_H
#define __DATA_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

#define LOG_RECORD_SIZE         16U                             /* bytes per record */
#define LOG_META_SECTOR_ADDR    0x000000UL                      /* Sector 0 base */
#define LOG_DATA_START_SECTOR   2U
#define LOG_DATA_START_ADDR     (LOG_DATA_START_SECTOR * 4096UL)
#define LOG_DATA_SECTOR_COUNT   4094U
#define LOG_DATA_END_ADDR       (LOG_DATA_START_ADDR + LOG_DATA_SECTOR_COUNT * 4096UL)
#define LOG_RECORDS_PER_SECTOR  256U                            /* 4096 / 16 */
#define LOG_MAX_RECORDS         (LOG_DATA_SECTOR_COUNT * LOG_RECORDS_PER_SECTOR)
#define LOG_PRE_ERASE_THRESHOLD 90U                             /* % fill before pre-erase */
#define LOG_KEY_LONG_PRESS_MS   1000U                           /* long-press duration (ms) */
#define LOG_META_MAGIC          0xA5C3B7E1UL                    /* metadata valid marker */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Log record type codes
 */
typedef enum {
    LOG_TYPE_BOOT         = 0x01U,  /* system power-on */
    LOG_TYPE_SELF_TEST_OK = 0x02U,  /* self-test passed */
    LOG_TYPE_SELF_TEST_NG = 0x03U,  /* self-test failed (param1 = error code) */
    LOG_TYPE_CH_OPEN      = 0x10U,  /* channel opened (param1 = ch 1/2/3) */
    LOG_TYPE_CH_CLOSE     = 0x11U,  /* channel closed (param1 = ch 1/2/3) */
    LOG_TYPE_ALARM_SET    = 0x20U,  /* alarm triggered (param1 = alarm type) */
    LOG_TYPE_ALARM_CLR    = 0x21U,  /* alarm cleared  (param1 = alarm type) */
    LOG_TYPE_FORMAT       = 0xF0U,  /* user-initiated Flash format */
} LogType_e;

/**
 * @brief 16-byte log record layout
 */
typedef struct {
    uint32_t timestamp;   /* bytes  0-3  : seconds since power-on */
    uint8_t  type;        /* byte   4    : LogType_e */
    uint8_t  param1;      /* byte   5    : type-specific parameter 1 */
    uint8_t  param2;      /* byte   6    : type-specific parameter 2 */
    uint8_t  reserved1;   /* byte   7    : 0x00 */
    uint32_t extra;       /* bytes  8-11 : additional data */
    uint8_t  reserved2;   /* byte  12    : 0x00 */
    uint8_t  reserved3;   /* byte  13    : 0x00 */
    uint8_t  reserved4;   /* byte  14    : 0x00 */
    uint8_t  crc8;        /* byte  15    : CRC8 over bytes 0..14 */
} LogRecord_t;

/**
 * @brief Metadata stored in Sector 0 (max 4096 bytes, currently ~24 bytes)
 */
typedef struct {
    uint32_t magic;               /* LOG_META_MAGIC validity marker */
    uint32_t write_ptr;           /* next write offset (bytes from LOG_DATA_START_ADDR) */
    uint32_t record_count;        /* cumulative records written (including overwrites) */
    uint32_t wrap_count;          /* number of circular wrap-arounds */
    uint8_t  pre_erased_sector;   /* last pre-erased sector index (relative to data area) */
    uint8_t  reserved[3];
} LogMeta_t;

/* Exported functions --------------------------------------------------------*/

bool DataLogger_Init(void);
void DataLogger_WriteChannelAction(uint8_t ch, bool open);
void DataLogger_WriteAlarm(uint16_t alarm_type, bool is_set);
void DataLogger_WriteBoot(void);
void DataLogger_KeyScan(void);
void DataLogger_BackgroundTask(void);
void DataLogger_TriggerDump(void);
void DataLogger_TriggerFormat(void);
bool DataLogger_IsReady(void);
bool DataLogger_IsOledLogActive(void);
void DataLogger_OledLogRefresh(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_LOGGER_H */

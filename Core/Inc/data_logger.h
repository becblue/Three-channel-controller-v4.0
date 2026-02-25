/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : data_logger.h
  * @brief          : 外部 Flash 数据日志模块头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-25
  ******************************************************************************
  * @note
  *   Flash 布局（W25Q128JVSIQ，16MB）：
  *     Sector 0  (0x000000, 4KB) ：元数据区（设备信息 + 循环缓冲区指针）
  *     Sector 1  (0x001000, 4KB) ：数据区起始（保留，方便扩展）
  *     Sector 2～4095            ：日志数据区（约 16MB - 8KB ≈ 16376 KB）
  *
  *   日志记录格式（16 字节固定长度）：
  *     Byte 0-3  : timestamp（ms，uint32_t，相对上电时间）
  *     Byte 4    : record_type（LOG_TYPE_xxx）
  *     Byte 5    : param1（type 相关参数）
  *     Byte 6    : param2（type 相关参数）
  *     Byte 7    : reserved（0x00）
  *     Byte 8-11 : extra（32 位附加信息，type 相关）
  *     Byte 12-14: reserved（0x00）
  *     Byte 15   : crc8（Byte 0-14 的 CRC8 校验）
  *
  *   循环存储策略：
  *     写指针满 90% 后，提前预擦除下一扇区，覆盖最旧记录。
  *
  *   按键操作（长按 3 秒）：
  *     KEY1（PC13）：串口输出所有日志（UART2，115200 bps）
  *     KEY2（PC14）：逻辑格式化（仅重置 Sector 0 元数据）
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

/** @brief 日志记录固定长度（字节）*/
#define LOG_RECORD_SIZE         16U

/** @brief 元数据扇区基地址 */
#define LOG_META_SECTOR_ADDR    0x000000UL

/** @brief 日志数据区起始扇区（Sector 2）*/
#define LOG_DATA_START_SECTOR   2U
#define LOG_DATA_START_ADDR     (LOG_DATA_START_SECTOR * 4096UL)

/** @brief 日志数据区结束地址（4096 扇区 - 2 = 4094 扇区用于数据）*/
#define LOG_DATA_SECTOR_COUNT   4094U
#define LOG_DATA_END_ADDR       (LOG_DATA_START_ADDR + LOG_DATA_SECTOR_COUNT * 4096UL)

/** @brief 每扇区可存记录数（4096 / 16 = 256）*/
#define LOG_RECORDS_PER_SECTOR  256U

/** @brief 总可存记录数 */
#define LOG_MAX_RECORDS         (LOG_DATA_SECTOR_COUNT * LOG_RECORDS_PER_SECTOR)

/** @brief 预擦除触发阈值（当前扇区写满 90% 后触发下一扇区预擦除）*/
#define LOG_PRE_ERASE_THRESHOLD 90U

/** @brief KEY 长按检测阈值（ms）*/
#define LOG_KEY_LONG_PRESS_MS   3000U

/** @brief 元数据魔数（用于有效性验证）*/
#define LOG_META_MAGIC          0xA5C3B7E1UL

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 日志记录类型枚举
 */
typedef enum {
    LOG_TYPE_BOOT         = 0x01U,  ///< 系统上电启动
    LOG_TYPE_SELF_TEST_OK = 0x02U,  ///< 自检通过
    LOG_TYPE_SELF_TEST_NG = 0x03U,  ///< 自检失败（param1=错误码）
    LOG_TYPE_CH_OPEN      = 0x10U,  ///< 通道打开（param1=通道号 1/2/3）
    LOG_TYPE_CH_CLOSE     = 0x11U,  ///< 通道关闭（param1=通道号 1/2/3）
    LOG_TYPE_ALARM_SET    = 0x20U,  ///< 报警触发（param1=报警类型）
    LOG_TYPE_ALARM_CLR    = 0x21U,  ///< 报警消除（param1=报警类型）
    LOG_TYPE_FORMAT       = 0xF0U,  ///< 用户执行 Flash 格式化
} LogType_e;

/**
 * @brief 日志记录结构体（对应 16 字节布局）
 */
typedef struct {
    uint32_t timestamp;   ///< Byte 0-3：上电经过 ms
    uint8_t  type;        ///< Byte 4：记录类型
    uint8_t  param1;      ///< Byte 5：参数 1
    uint8_t  param2;      ///< Byte 6：参数 2
    uint8_t  reserved1;   ///< Byte 7：保留（0x00）
    uint32_t extra;       ///< Byte 8-11：附加数据
    uint8_t  reserved2;   ///< Byte 12：保留（0x00）
    uint8_t  reserved3;   ///< Byte 13：保留（0x00）
    uint8_t  reserved4;   ///< Byte 14：保留（0x00）
    uint8_t  crc8;        ///< Byte 15：CRC8 校验（覆盖 Byte 0-14）
} LogRecord_t;

/**
 * @brief 元数据结构体（存储于 Sector 0，4KB）
 * @note  总大小需 ≤ 4096 字节；当前约 24 字节
 */
typedef struct {
    uint32_t magic;          ///< 魔数 LOG_META_MAGIC（有效性标志）
    uint32_t write_ptr;      ///< 下一条记录的写入地址（字节偏移，相对 LOG_DATA_START_ADDR）
    uint32_t record_count;   ///< 历史累计写入记录总数（含覆盖写）
    uint32_t wrap_count;     ///< 已循环覆盖次数
    uint8_t  pre_erased_sector; ///< 已预擦除的下一扇区号（相对 LOG_DATA_START_SECTOR）
    uint8_t  reserved[3];    ///< 对齐保留
} LogMeta_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  日志模块初始化（读取元数据，验证 Flash 就绪）
 * @note   必须在 W25Q_Init() 之后调用
 * @retval true=初始化成功，false=Flash 异常（日志功能不可用）
 */
bool DataLogger_Init(void);

/**
 * @brief  写入通道动作记录
 * @param  ch:   通道号（1/2/3）
 * @param  open: true=打开，false=关闭
 */
void DataLogger_WriteChannelAction(uint8_t ch, bool open);

/**
 * @brief  写入报警事件记录
 * @param  alarm_type:  报警类型代码（来自 alarm_output.h 的枚举值）
 * @param  is_set:      true=触发，false=消除
 */
void DataLogger_WriteAlarm(uint8_t alarm_type, bool is_set);

/**
 * @brief  写入系统启动记录（在 DataLogger_Init 内部自动调用）
 */
void DataLogger_WriteBoot(void);

/**
 * @brief  按键轮询扫描（需在 20ms 任务中调用）
 * @note   检测 KEY1(PC13) 和 KEY2(PC14) 的长按（3 秒）
 */
void DataLogger_KeyScan(void);

/**
 * @brief  后台任务（需在 100ms 任务中调用）
 * @note   处理异步预擦除、串口输出等耗时操作分片
 */
void DataLogger_BackgroundTask(void);

/**
 * @brief  触发串口日志转储（由 KEY1 长按触发，或外部调用）
 * @note   在 DataLogger_BackgroundTask 中异步分批输出
 */
void DataLogger_TriggerDump(void);

/**
 * @brief  触发逻辑格式化（由 KEY2 长按触发）
 * @note   仅重置 Sector 0 元数据，不擦除全片
 */
void DataLogger_TriggerFormat(void);

/**
 * @brief  获取日志模块是否可用
 * @retval true=可用，false=Flash 异常
 */
bool DataLogger_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_LOGGER_H */

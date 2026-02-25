/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : w25q_flash.h
  * @brief          : W25Q128JVSIQ SPI NOR Flash 底层驱动头文件
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-25
  ******************************************************************************
  * @note
  *   硬件连接：SPI2（PB12=NSS, PB13=SCK, PB14=MISO, PB15=MOSI）
  *   芯片规格：128Mbit = 16MB，4096 扇区 × 4KB，256 块 × 64KB
  *   SPI 模式：Mode 0（CPOL=0, CPHA=0），最高 133MHz
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __W25Q_FLASH_H
#define __W25Q_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @brief Flash 容量参数 */
#define W25Q_TOTAL_SIZE         (16UL * 1024UL * 1024UL) ///< 总容量 16MB
#define W25Q_SECTOR_SIZE        4096U                     ///< 扇区大小 4KB
#define W25Q_PAGE_SIZE          256U                      ///< 页大小 256B
#define W25Q_SECTOR_COUNT       4096U                     ///< 扇区总数

/** @brief JEDEC ID（W25Q128JV-IQ/IN/JQ）*/
#define W25Q_MANUFACTURER_ID    0xEFU   ///< Winbond 制造商 ID
#define W25Q_DEVICE_ID_HI       0x40U   ///< 器件 ID 高字节（Memory Type）
#define W25Q_DEVICE_ID_LO       0x18U   ///< 器件 ID 低字节（Capacity）

/** @brief 指令集（Standard SPI）*/
#define W25Q_CMD_WRITE_ENABLE   0x06U   ///< 写使能
#define W25Q_CMD_WRITE_DISABLE  0x04U   ///< 写禁止
#define W25Q_CMD_READ_SR1       0x05U   ///< 读状态寄存器 1
#define W25Q_CMD_READ_DATA      0x03U   ///< 读数据（≤50MHz）
#define W25Q_CMD_PAGE_PROGRAM   0x02U   ///< 页编程（最多 256 字节）
#define W25Q_CMD_SECTOR_ERASE   0x20U   ///< 扇区擦除（4KB）
#define W25Q_CMD_CHIP_ERASE     0xC7U   ///< 全片擦除（仅格式化用）
#define W25Q_CMD_JEDEC_ID       0x9FU   ///< 读 JEDEC ID

/** @brief 状态寄存器 SR1 位掩码 */
#define W25Q_SR1_BUSY           0x01U   ///< Bit0：忙标志（1=正在编程/擦除）
#define W25Q_SR1_WEL            0x02U   ///< Bit1：写使能锁存

/** @brief 操作超时（ms，基于最长规格 + 裕量）*/
#define W25Q_TIMEOUT_PAGE_PROG  10U     ///< 页编程超时（最长 3ms，取 10ms）
#define W25Q_TIMEOUT_SECTOR_ERA 500U    ///< 扇区擦除超时（最长 400ms，取 500ms）
#define W25Q_TIMEOUT_READ       50U     ///< 读操作超时

/* Exported types ------------------------------------------------------------*/

/**
 * @brief W25Q Flash 操作结果
 */
typedef enum {
    W25Q_OK       = 0,  ///< 操作成功
    W25Q_ERROR    = 1,  ///< 操作失败（SPI 通信错误）
    W25Q_TIMEOUT  = 2,  ///< 等待 BUSY 超时
    W25Q_ID_ERROR = 3,  ///< JEDEC ID 不匹配
} W25Q_Result_e;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Flash 模块初始化（读 JEDEC ID 验证芯片）
 * @retval W25Q_OK / W25Q_ID_ERROR / W25Q_ERROR
 */
W25Q_Result_e W25Q_Init(void);

/**
 * @brief  读取数据
 * @param  addr:  Flash 起始地址（0x000000 ~ 0xFFFFFF）
 * @param  buf:   目标缓冲区
 * @param  len:   读取字节数
 * @retval W25Q_OK / W25Q_ERROR
 */
W25Q_Result_e W25Q_Read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief  页编程（最多 256 字节，目标区域须为已擦除状态 0xFF）
 * @param  addr:  Flash 页对齐地址
 * @param  buf:   源数据缓冲区
 * @param  len:   写入字节数（≤256）
 * @retval W25Q_OK / W25Q_ERROR / W25Q_TIMEOUT
 */
W25Q_Result_e W25Q_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len);

/**
 * @brief  扇区擦除（4KB，阻塞等待完成）
 * @param  sector_addr:  扇区内任意地址（自动对齐到 4KB 边界）
 * @retval W25Q_OK / W25Q_ERROR / W25Q_TIMEOUT
 */
W25Q_Result_e W25Q_SectorErase(uint32_t sector_addr);

/**
 * @brief  等待 Flash 空闲（轮询 BUSY 位）
 * @param  timeout_ms:  超时时间（ms）
 * @retval W25Q_OK / W25Q_TIMEOUT / W25Q_ERROR
 */
W25Q_Result_e W25Q_WaitBusy(uint32_t timeout_ms);

/**
 * @brief  检查指定扇区是否全为 0xFF（空白）
 * @param  sector_addr:  扇区起始地址
 * @retval true=空白，false=有数据
 */
bool W25Q_IsSectorBlank(uint32_t sector_addr);

#ifdef __cplusplus
}
#endif

#endif /* __W25Q_FLASH_H */

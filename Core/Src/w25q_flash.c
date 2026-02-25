/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : w25q_flash.c
  * @brief          : W25Q128JVSIQ SPI NOR Flash low-level driver
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-25
  ******************************************************************************
 * @note
 *   SPI2 uses software NSS: PB12 is driven by this driver (NSS low during transfer).
 *   All write operations include internal WriteEnable sequence.
 *   SectorErase is a blocking call (up to 400 ms).
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "w25q_flash.h"
#include "spi.h"
#include "stm32f1xx_hal.h"

/* Private defines -----------------------------------------------------------*/
#define SPI_BYTE_TIMEOUT_MS     5U   /* per-byte SPI timeout (ms) */
#define W25Q_NSS_PORT          GPIOB
#define W25Q_NSS_PIN           GPIO_PIN_12
#define W25Q_NSS_LOW()         HAL_GPIO_WritePin(W25Q_NSS_PORT, W25Q_NSS_PIN, GPIO_PIN_RESET)
#define W25Q_NSS_HIGH()        HAL_GPIO_WritePin(W25Q_NSS_PORT, W25Q_NSS_PIN, GPIO_PIN_SET)

/* Private variables ---------------------------------------------------------*/
static uint8_t s_tx_buf[4];   /* shared cmd+addr TX buffer (max 4 bytes) */

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef spi_transmit(const uint8_t *tx, uint16_t len);
static HAL_StatusTypeDef spi_transceive(const uint8_t *tx, uint8_t *rx, uint16_t len);
static W25Q_Result_e     send_write_enable(void);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Flash init: verify JEDEC ID (0xEF 0x40 0x18)
  */
W25Q_Result_e W25Q_Init(void)
{
    /* Send 0x9F + 3 dummy bytes, receive manufacturer/type/capacity */
    uint8_t tx4[4] = {W25Q_CMD_JEDEC_ID, 0xFFU, 0xFFU, 0xFFU};
    uint8_t rx4[4] = {0};

    W25Q_NSS_LOW();
    if (spi_transceive(tx4, rx4, 4U) != HAL_OK) {
        W25Q_NSS_HIGH();
        return W25Q_ERROR;
    }
    W25Q_NSS_HIGH();

    if ((rx4[1] != W25Q_MANUFACTURER_ID) ||
        (rx4[2] != W25Q_DEVICE_ID_HI)    ||
        (rx4[3] != W25Q_DEVICE_ID_LO)) {
        return W25Q_ID_ERROR;
    }
    return W25Q_OK;
}

/**
  * @brief  Read data from Flash (any length, auto-split into 256-byte chunks)
  * @note   Software NSS: NSS low, then Transmit(cmd+addr 4 bytes), then Receive(data).
  *         First byte received is D0; no phase ambiguity.
  */
W25Q_Result_e W25Q_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t remain = len;
    uint32_t offset = 0U;
    uint16_t chunk;
    uint32_t timeout;

    if ((buf == NULL) || (len == 0U)) {
        return W25Q_ERROR;
    }

    while (remain > 0U) {
        chunk = (remain > 256U) ? 256U : (uint16_t)remain;
        timeout = (uint32_t)chunk * SPI_BYTE_TIMEOUT_MS;
        if (timeout < 50U) { timeout = 50U; }

        s_tx_buf[0] = W25Q_CMD_READ_DATA;
        s_tx_buf[1] = (uint8_t)((addr >> 16U) & 0xFFU);
        s_tx_buf[2] = (uint8_t)((addr >>  8U) & 0xFFU);
        s_tx_buf[3] = (uint8_t)( addr         & 0xFFU);

        W25Q_NSS_LOW();
        if (spi_transmit(s_tx_buf, 4U) != HAL_OK) {
            W25Q_NSS_HIGH();
            return W25Q_ERROR;
        }
        if (HAL_SPI_Receive(&hspi2, buf + offset, chunk, timeout) != HAL_OK) {
            W25Q_NSS_HIGH();
            return W25Q_ERROR;
        }
        W25Q_NSS_HIGH();

        offset += chunk;
        remain -= chunk;
        addr   += chunk;
    }

    return W25Q_OK;
}

/**
  * @brief  Sector erase (4 KB, blocking up to 400 ms)
  */
W25Q_Result_e W25Q_SectorErase(uint32_t sector_addr)
{
    uint32_t aligned;

    if (send_write_enable() != W25Q_OK) {
        return W25Q_ERROR;
    }

    aligned = sector_addr & ~(W25Q_SECTOR_SIZE - 1U);  /* align to 4 KB */

    s_tx_buf[0] = W25Q_CMD_SECTOR_ERASE;
    s_tx_buf[1] = (uint8_t)((aligned >> 16U) & 0xFFU);
    s_tx_buf[2] = (uint8_t)((aligned >>  8U) & 0xFFU);
    s_tx_buf[3] = (uint8_t)( aligned         & 0xFFU);

    W25Q_NSS_LOW();
    if (spi_transmit(s_tx_buf, 4U) != HAL_OK) {
        W25Q_NSS_HIGH();
        return W25Q_ERROR;
    }
    W25Q_NSS_HIGH();

    return W25Q_WaitBusy(W25Q_TIMEOUT_SECTOR_ERA);
}

/**
  * @brief  Page program (max 256 bytes, target area must be blank 0xFF)
  */
W25Q_Result_e W25Q_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    uint8_t frame[4U + W25Q_PAGE_SIZE];

    if ((buf == NULL) || (len == 0U) || (len > W25Q_PAGE_SIZE)) {
        return W25Q_ERROR;
    }

    if (send_write_enable() != W25Q_OK) {
        return W25Q_ERROR;
    }

    frame[0] = W25Q_CMD_PAGE_PROGRAM;
    frame[1] = (uint8_t)((addr >> 16U) & 0xFFU);
    frame[2] = (uint8_t)((addr >>  8U) & 0xFFU);
    frame[3] = (uint8_t)( addr         & 0xFFU);
    for (uint16_t i = 0U; i < len; i++) {
        frame[4U + i] = buf[i];
    }

    W25Q_NSS_LOW();
    if (spi_transmit(frame, (uint16_t)(4U + len)) != HAL_OK) {
        W25Q_NSS_HIGH();
        return W25Q_ERROR;
    }
    W25Q_NSS_HIGH();

    return W25Q_WaitBusy(W25Q_TIMEOUT_PAGE_PROG);
}

/**
  * @brief  Poll SR1 BUSY bit until Flash is idle or timeout
  */
W25Q_Result_e W25Q_WaitBusy(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t  tx[2] = {W25Q_CMD_READ_SR1, 0xFFU};
    uint8_t  rx[2];

    while (1) {
        W25Q_NSS_LOW();
        if (spi_transceive(tx, rx, 2U) != HAL_OK) {
            W25Q_NSS_HIGH();
            return W25Q_ERROR;
        }
        W25Q_NSS_HIGH();
        if ((rx[1] & W25Q_SR1_BUSY) == 0U) {
            return W25Q_OK;
        }
        if ((HAL_GetTick() - start) >= timeout_ms) {
            return W25Q_TIMEOUT;
        }
        HAL_Delay(1U);
    }
}

/**
  * @brief  Check whether a 4 KB sector is entirely 0xFF (blank)
  */
bool W25Q_IsSectorBlank(uint32_t sector_addr)
{
    uint8_t  page_buf[256];
    uint32_t aligned = sector_addr & ~(W25Q_SECTOR_SIZE - 1U);
    uint8_t  pages   = (uint8_t)(W25Q_SECTOR_SIZE / W25Q_PAGE_SIZE);  /* 16 */

    for (uint8_t p = 0U; p < pages; p++) {
        uint32_t page_addr = aligned + (uint32_t)p * W25Q_PAGE_SIZE;
        if (W25Q_Read(page_addr, page_buf, W25Q_PAGE_SIZE) != W25Q_OK) {
            return false;
        }
        for (uint16_t i = 0U; i < W25Q_PAGE_SIZE; i++) {
            if (page_buf[i] != 0xFFU) {
                return false;
            }
        }
    }
    return true;
}

/* Private functions ---------------------------------------------------------*/

static HAL_StatusTypeDef spi_transmit(const uint8_t *tx, uint16_t len)
{
    return HAL_SPI_Transmit(&hspi2, (uint8_t *)tx, len, SPI_BYTE_TIMEOUT_MS * len);
}

static HAL_StatusTypeDef spi_transceive(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    return HAL_SPI_TransmitReceive(&hspi2, (uint8_t *)tx, rx, len,
                                   SPI_BYTE_TIMEOUT_MS * len);
}

static W25Q_Result_e send_write_enable(void)
{
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    W25Q_NSS_LOW();
    if (spi_transmit(&cmd, 1U) != HAL_OK) {
        W25Q_NSS_HIGH();
        return W25Q_ERROR;
    }
    W25Q_NSS_HIGH();
    return W25Q_OK;
}

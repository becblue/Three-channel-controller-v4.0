/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : w25q_flash.c
  * @brief          : W25Q128JVSIQ SPI NOR Flash 搴曞眰椹卞姩瀹炵�?
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-25
  ******************************************************************************
  * @note
  *   SPI2 纭�?NSS锛圥B12锛夌�?HAL 鑷姩鎷変綆/閲婃斁锛屾棤闇€鎵嬪姩鎿嶄綔 CS 寮曡剼銆�?
  *   鎵€鏈夊啓鎿嶄綔锛圥ageProgram / SectorErase锛夊唴閮ㄥ凡鍖呭�?WriteEnable 娴佺▼銆�?
  *   鎵囧尯鎿﹂櫎涓洪樆濉炴搷浣滐紙鏈€闀�?400ms锛夛紝璋冪敤鏂归』纭鏃跺簭鍏佽銆�
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "w25q_flash.h"
#include "spi.h"
#include "stm32f1xx_hal.h"

/* Private defines -----------------------------------------------------------*/
/** @brief SPI 鍗曞瓧鑺傛敹鍙戣秴鏃讹紙ms锛�*/
#define SPI_BYTE_TIMEOUT_MS     5U

/* Private variables ---------------------------------------------------------*/
static uint8_t s_tx_buf[4];   ///< 鍛戒�?鍦板潃鍙戦€佺紦鍐插尯锛堟渶澶� 4 瀛楄妭锛�?
static uint8_t s_rx_buf[4];   ///< 鐘舵€�/ID 鎺ユ敹缂撳啿鍖�

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef spi_transmit(const uint8_t *tx, uint16_t len);
static HAL_StatusTypeDef spi_transceive(const uint8_t *tx, uint8_t *rx, uint16_t len);
static W25Q_Result_e     send_write_enable(void);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Flash 鍒濆鍖栵細璇诲�?JEDEC ID 楠岃瘉鑺墖鍨嬪�?
  */
W25Q_Result_e W25Q_Init(void)
{
    /* 璇� JEDEC ID锛氬彂閫�?0x9F锛屾帴鏀�?3 瀛楄妭锛圡anufID + MemType + Capacity锛�*/
    s_tx_buf[0] = W25Q_CMD_JEDEC_ID;
    uint8_t id[3] = {0};

    HAL_StatusTypeDef ret;
    /* NSS 鐢� HAL 纭欢鎺у埗锛屾暣甯у彂閫侊細鍛戒护锛�1B锛�+ 鎺ユ敹锛�?B锛�*/
    uint8_t tx4[4] = {W25Q_CMD_JEDEC_ID, 0xFF, 0xFF, 0xFF};
    uint8_t rx4[4] = {0};
    ret = spi_transceive(tx4, rx4, 4U);
    if (ret != HAL_OK) {
        return W25Q_ERROR;
    }
    id[0] = rx4[1]; /* 鍒堕€犲�?ID */
    id[1] = rx4[2]; /* Memory Type */
    id[2] = rx4[3]; /* Capacity */

    if ((id[0] != W25Q_MANUFACTURER_ID) ||
        (id[1] != W25Q_DEVICE_ID_HI)    ||
        (id[2] != W25Q_DEVICE_ID_LO)) {
        return W25Q_ID_ERROR;
    }
    return W25Q_OK;
}

/**
  * @brief  璇诲�?Flash 鏁版�?
  */
W25Q_Result_e W25Q_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    if ((buf == NULL) || (len == 0U)) {
        return W25Q_ERROR;
    }

    /* 缁勫抚锛氬懡浠�(1B) + 鍦板�?3B) = 4 瀛楄妭澶达紝鍚庤�?len 瀛楄妭铏氭嫙 TX */
    uint8_t header[4];
    header[0] = W25Q_CMD_READ_DATA;
    header[1] = (uint8_t)((addr >> 16U) & 0xFFU);
    header[2] = (uint8_t)((addr >>  8U) & 0xFFU);
    header[3] = (uint8_t)( addr         & 0xFFU);

    /* 鍙戦€佸懡浠�?鍦板潃澶达紙鍚屾椂鎺ユ敹鍒扮�?4 瀛楄妭涓烘棤鏁堟暟鎹紝涓㈠純锛�?/
    uint8_t dummy4[4] = {0};
    if (spi_transceive(header, dummy4, 4U) != HAL_OK) {
        return W25Q_ERROR;
    }

    /* 鎸佺画鍙戦€�?0xFF dummy 浠ラ┍鍔ㄦ椂閽燂紝鍚屾椂鎺ユ敹鏈夋晥鏁版�?*/
    /* HAL_SPI_TransmitReceive 瑕佹�?TX/RX 闀垮害鐩稿�?*/
    /* 鍒嗘壒鏈€澶�?256 瀛楄妭锛堜竴椤碉級閬垮厤鏍堜笂澶ф暟缁�?*/
    uint32_t remain = len;
    uint32_t offset = 0U;
    uint8_t  dummy_tx[256];
    uint8_t  rx_buf[256];

    /* 棰勫～鍏�?dummy TX = 0xFF */
    for (uint16_t i = 0U; i < 256U; i++) {
        dummy_tx[i] = 0xFFU;
    }

    while (remain > 0U) {
        uint16_t chunk = (remain > 256U) ? 256U : (uint16_t)remain;
        if (HAL_SPI_TransmitReceive(&hspi2, dummy_tx, rx_buf, chunk,
                                    W25Q_TIMEOUT_READ) != HAL_OK) {
            return W25Q_ERROR;
        }
        for (uint16_t i = 0U; i < chunk; i++) {
            buf[offset + i] = rx_buf[i];
        }
        offset += chunk;
        remain -= chunk;
    }

    /* 娉ㄦ剰锛氱‖浠�?NSS 鍦ㄦ暣涓�?HAL_SPI_Init 瀹屾垚鍚庢槸鐢辩‖浠剁鐞嗙殑锛�?
     * 鐢变簬浣跨敤 SPI_NSS_HARD_OUTPUT锛孨SS 鍦� SPI 妯″潡浣胯兘鏃舵媺浣庯�?
     * 浣嗗疄闄呬笂姣忔�?HAL_SPI_TransmitReceive 骞朵笉浼氳嚜鍔ㄦ媺浣�?閲婃�?NSS銆�
     * STM32F1 纭�?NSS 浠呭�?SPI 鏁版嵁甯х粨鏉熷悗锛圱XE+BSY=0锛夐噴鏀俱€�?
     * 瀵逛簬 Flash 鐨勫瀛楄妭杩炵画璇伙紝闇€淇濇寔 CS 浣庣洿鍒拌瀹岋�?
     * 鐢变簬鍒嗘壒璋冪�?HAL_SPI_TransmitReceive锛屼腑闂�?NSS 鍙兘閲婃斁锛堢‖浠�?SSM=0 鏃讹級銆�?
     * 瀹為檯娴嬭瘯鑻ユ湁闂锛屽彲灏� NSS 鏀逛负杞欢鎺у�?(SPI_NSS_SOFT)銆�
     */
    return W25Q_OK;
}

/**
  * @brief  鎵囧尯鎿﹂櫎锛�4KB锛�
  */
W25Q_Result_e W25Q_SectorErase(uint32_t sector_addr)
{
    /* 鍐欎娇鑳�?*/
    if (send_write_enable() != W25Q_OK) {
        return W25Q_ERROR;
    }

    /* 鍦板潃瀵归綈鍒�?4KB 杈圭�?*/
    uint32_t aligned = sector_addr & ~(W25Q_SECTOR_SIZE - 1U);

    s_tx_buf[0] = W25Q_CMD_SECTOR_ERASE;
    s_tx_buf[1] = (uint8_t)((aligned >> 16U) & 0xFFU);
    s_tx_buf[2] = (uint8_t)((aligned >>  8U) & 0xFFU);
    s_tx_buf[3] = (uint8_t)( aligned         & 0xFFU);

    if (spi_transmit(s_tx_buf, 4U) != HAL_OK) {
        return W25Q_ERROR;
    }

    /* 闃诲绛夊緟鎿﹂櫎瀹屾垚锛堟渶闀�?400ms锛�*/
    return W25Q_WaitBusy(W25Q_TIMEOUT_SECTOR_ERA);
}

/**
  * @brief  椤电紪绋嬶紙鏈€澶� 256 瀛楄妭锛岀洰鏍囧尯鍩熼』涓�?0xFF锛�
  */
W25Q_Result_e W25Q_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len == 0U) || (len > W25Q_PAGE_SIZE)) {
        return W25Q_ERROR;
    }

    /* 鍐欎娇鑳�?*/
    if (send_write_enable() != W25Q_OK) {
        return W25Q_ERROR;
    }

    /* 缁勫抚锛氬懡浠�(1B) + 鍦板�?3B) + 鏁版�?len B)锛屾渶澶�?260 瀛楄�?*/
    uint8_t frame[4U + W25Q_PAGE_SIZE];
    frame[0] = W25Q_CMD_PAGE_PROGRAM;
    frame[1] = (uint8_t)((addr >> 16U) & 0xFFU);
    frame[2] = (uint8_t)((addr >>  8U) & 0xFFU);
    frame[3] = (uint8_t)( addr         & 0xFFU);
    for (uint16_t i = 0U; i < len; i++) {
        frame[4U + i] = buf[i];
    }

    if (spi_transmit(frame, (uint16_t)(4U + len)) != HAL_OK) {
        return W25Q_ERROR;
    }

    /* 闃诲绛夊緟缂栫▼瀹屾垚锛堟渶闀�?3ms锛�*/
    return W25Q_WaitBusy(W25Q_TIMEOUT_PAGE_PROG);
}

/**
  * @brief  杞绛夊緟 Flash BUSY 浣嶆竻闆�?
  */
W25Q_Result_e W25Q_WaitBusy(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t  tx[2] = {W25Q_CMD_READ_SR1, 0xFFU};
    uint8_t  rx[2];

    while (1) {
        if (spi_transceive(tx, rx, 2U) != HAL_OK) {
            return W25Q_ERROR;
        }
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
  * @brief  妫€鏌ユ墖鍖烘槸鍚﹀叏涓�?0xFF锛堢┖鐧斤級
  */
bool W25Q_IsSectorBlank(uint32_t sector_addr)
{
    uint8_t buf[256];
    uint32_t aligned = sector_addr & ~(W25Q_SECTOR_SIZE - 1U);
    uint8_t  pages   = (uint8_t)(W25Q_SECTOR_SIZE / W25Q_PAGE_SIZE); /* 16 椤� */

    for (uint8_t p = 0U; p < pages; p++) {
        uint32_t page_addr = aligned + (uint32_t)p * W25Q_PAGE_SIZE;
        if (W25Q_Read(page_addr, buf, W25Q_PAGE_SIZE) != W25Q_OK) {
            return false;
        }
        for (uint16_t i = 0U; i < W25Q_PAGE_SIZE; i++) {
            if (buf[i] != 0xFFU) {
                return false;
            }
        }
    }
    return true;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  SPI 绾彂閫侊紙浠� TX锛屼笉鍏冲績 RX锛�
  */
static HAL_StatusTypeDef spi_transmit(const uint8_t *tx, uint16_t len)
{
    return HAL_SPI_Transmit(&hspi2, (uint8_t *)tx, len, SPI_BYTE_TIMEOUT_MS * len);
}


/**
  * @brief  SPI 鍏ㄥ弻宸ユ敹鍙戯紙TX 鍜� RX 绛夐暱锛�?
  */
static HAL_StatusTypeDef spi_transceive(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    return HAL_SPI_TransmitReceive(&hspi2, (uint8_t *)tx, rx, len,
                                   SPI_BYTE_TIMEOUT_MS * len);
}

/**
  * @brief  鍙戦€佸啓浣胯兘鍛戒护锛堟墍鏈夊啓鎿嶄綔鍓嶈皟鐢級
  */
static W25Q_Result_e send_write_enable(void)
{
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    if (spi_transmit(&cmd, 1U) != HAL_OK) {
        return W25Q_ERROR;
    }
    return W25Q_OK;
}

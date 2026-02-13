/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : oled_display.c
  * @brief          : OLED display driver implementation (SSD1309)
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-13
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "oled_display.h"
#include "i2c.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define OLED_CMD_MODE   0x00    // Command mode
#define OLED_DATA_MODE  0x40    // Data mode
#define OLED_TIMEOUT    100     // I2C timeout (ms)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// Display buffer (128x64 = 1024 bytes)
static uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];

/* Private function prototypes -----------------------------------------------*/
static bool oled_write_cmd(uint8_t cmd);
static bool oled_write_data(uint8_t *data, uint16_t len);

/* Font data (6x8 ASCII) -----------------------------------------------------*/
static const uint8_t font_6x8[][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // sp (space)
    {0x00, 0x00, 0x2f, 0x00, 0x00, 0x00},   // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00},   // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14, 0x00},   // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x00},   // $
    {0x23, 0x13, 0x08, 0x64, 0x62, 0x00},   // %
    {0x36, 0x49, 0x55, 0x22, 0x50, 0x00},   // &
    {0x00, 0x05, 0x03, 0x00, 0x00, 0x00},   // '
    {0x00, 0x1c, 0x22, 0x41, 0x00, 0x00},   // (
    {0x00, 0x41, 0x22, 0x1c, 0x00, 0x00},   // )
    {0x14, 0x08, 0x3E, 0x08, 0x14, 0x00},   // *
    {0x08, 0x08, 0x3E, 0x08, 0x08, 0x00},   // +
    {0x00, 0x00, 0xA0, 0x60, 0x00, 0x00},   // ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x00},   // -
    {0x00, 0x60, 0x60, 0x00, 0x00, 0x00},   // .
    {0x20, 0x10, 0x08, 0x04, 0x02, 0x00},   // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00},   // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00},   // 1
    {0x42, 0x61, 0x51, 0x49, 0x46, 0x00},   // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00},   // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00},   // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00},   // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00},   // 6
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00},   // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00},   // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00},   // 9
    {0x00, 0x36, 0x36, 0x00, 0x00, 0x00},   // :
    {0x00, 0x56, 0x36, 0x00, 0x00, 0x00},   // ;
    {0x08, 0x14, 0x22, 0x41, 0x00, 0x00},   // <
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x00},   // =
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00},   // >
    {0x02, 0x01, 0x51, 0x09, 0x06, 0x00},   // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E, 0x00},   // @
    {0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00},   // A
    {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00},   // B
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00},   // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00},   // D
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00},   // E
    {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00},   // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00},   // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00},   // H
    {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00},   // I
    {0x20, 0x40, 0x41, 0x3F, 0x01, 0x00},   // J
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00},   // K
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00},   // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00},   // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00},   // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00},   // O
    {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00},   // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00},   // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00},   // R
    {0x46, 0x49, 0x49, 0x49, 0x31, 0x00},   // S
    {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00},   // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00},   // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00},   // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00},   // W
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00},   // X
    {0x07, 0x08, 0x70, 0x08, 0x07, 0x00},   // Y
    {0x61, 0x51, 0x49, 0x45, 0x43, 0x00},   // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00, 0x00},   // [
    {0x02, 0x04, 0x08, 0x10, 0x20, 0x00},   // '\'
    {0x00, 0x41, 0x41, 0x7F, 0x00, 0x00},   // ]
    {0x04, 0x02, 0x01, 0x02, 0x04, 0x00},   // ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x00},   // _
    {0x00, 0x01, 0x02, 0x04, 0x00, 0x00},   // '
    {0x20, 0x54, 0x54, 0x54, 0x78, 0x00},   // a
    {0x7F, 0x48, 0x44, 0x44, 0x38, 0x00},   // b
    {0x38, 0x44, 0x44, 0x44, 0x20, 0x00},   // c
    {0x38, 0x44, 0x44, 0x48, 0x7F, 0x00},   // d
    {0x38, 0x54, 0x54, 0x54, 0x18, 0x00},   // e
    {0x08, 0x7E, 0x09, 0x01, 0x02, 0x00},   // f
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C, 0x00},   // g
    {0x7F, 0x08, 0x04, 0x04, 0x78, 0x00},   // h
    {0x00, 0x44, 0x7D, 0x40, 0x00, 0x00},   // i
    {0x40, 0x80, 0x84, 0x7D, 0x00, 0x00},   // j
    {0x7F, 0x10, 0x28, 0x44, 0x00, 0x00},   // k
    {0x00, 0x41, 0x7F, 0x40, 0x00, 0x00},   // l
    {0x7C, 0x04, 0x18, 0x04, 0x78, 0x00},   // m
    {0x7C, 0x08, 0x04, 0x04, 0x78, 0x00},   // n
    {0x38, 0x44, 0x44, 0x44, 0x38, 0x00},   // o
    {0xFC, 0x24, 0x24, 0x24, 0x18, 0x00},   // p
    {0x18, 0x24, 0x24, 0x18, 0xFC, 0x00},   // q
    {0x7C, 0x08, 0x04, 0x04, 0x08, 0x00},   // r
    {0x48, 0x54, 0x54, 0x54, 0x20, 0x00},   // s
    {0x04, 0x3F, 0x44, 0x40, 0x20, 0x00},   // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00},   // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00},   // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00},   // w
    {0x44, 0x28, 0x10, 0x28, 0x44, 0x00},   // x
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C, 0x00},   // y
    {0x44, 0x64, 0x54, 0x4C, 0x44, 0x00},   // z
    {0x00, 0x10, 0x7C, 0x82, 0x00, 0x00},   // {
    {0x00, 0x00, 0xFF, 0x00, 0x00, 0x00},   // |
    {0x00, 0x82, 0x7C, 0x10, 0x00, 0x00},   // }
    {0x00, 0x06, 0x09, 0x09, 0x06, 0x00}    // ~ (degree symbol)
};

/* Exported functions --------------------------------------------------------*/

bool OLED_Init(void)
{
    printf("[OLED] Initializing display...\r\n");
    
    // Wait for OLED power stabilization
    HAL_Delay(100);
    
    // Initialization sequence for SSD1309
    oled_write_cmd(0xAE);  // Display OFF
    oled_write_cmd(0xD5);  // Set display clock divide ratio/oscillator frequency
    oled_write_cmd(0x80);  // Default setting
    oled_write_cmd(0xA8);  // Set multiplex ratio
    oled_write_cmd(0x3F);  // 1/64 duty
    oled_write_cmd(0xD3);  // Set display offset
    oled_write_cmd(0x00);  // No offset
    oled_write_cmd(0x40);  // Set start line address
    oled_write_cmd(0x8D);  // Charge pump setting
    oled_write_cmd(0x14);  // Enable charge pump
    oled_write_cmd(0x20);  // Set memory addressing mode
    oled_write_cmd(0x00);  // Horizontal addressing mode
    oled_write_cmd(0xA1);  // Set segment re-map (A0/A1)
    oled_write_cmd(0xC8);  // Set COM output scan direction (C0/C8)
    oled_write_cmd(0xDA);  // Set COM pins hardware configuration
    oled_write_cmd(0x12);  // Alternative COM pin config
    oled_write_cmd(0x81);  // Set contrast control
    oled_write_cmd(0xCF);  // Medium brightness
    oled_write_cmd(0xD9);  // Set pre-charge period
    oled_write_cmd(0xF1);  // Phase 1: 15 DCLK, Phase 2: 1 DCLK
    oled_write_cmd(0xDB);  // Set VCOMH deselect level
    oled_write_cmd(0x40);  // ~0.77xVcc
    oled_write_cmd(0xA4);  // Entire display ON (A4=follow RAM, A5=ignore RAM)
    oled_write_cmd(0xA6);  // Set normal display (A6=normal, A7=inverse)
    oled_write_cmd(0xAF);  // Display ON
    
    // Clear buffer and display
    memset(oled_buffer, 0, sizeof(oled_buffer));
    OLED_Refresh();
    
    printf("[OLED] Initialization complete\r\n");
    
    return true;
}

void OLED_Clear(void)
{
    memset(oled_buffer, 0, sizeof(oled_buffer));
}

void OLED_ClearArea(OLED_Area_e area)
{
    uint8_t start_page, end_page;
    
    switch(area) {
        case OLED_AREA_ALARM:
            start_page = OLED_AREA_ALARM_START;
            end_page = OLED_AREA_ALARM_END;
            break;
        case OLED_AREA_CHANNEL:
            start_page = OLED_AREA_CHANNEL_START;
            end_page = OLED_AREA_CHANNEL_END;
            break;
        case OLED_AREA_TEMP:
            start_page = OLED_AREA_TEMP_START;
            end_page = OLED_AREA_TEMP_END;
            break;
        default:
            return;
    }
    
    for(uint8_t page = start_page; page <= end_page; page++) {
        memset(&oled_buffer[page * OLED_WIDTH], 0, OLED_WIDTH);
    }
}

void OLED_DrawPixel(uint8_t x, uint8_t y, bool on)
{
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    uint16_t idx = page * OLED_WIDTH + x;
    
    if(on) {
        oled_buffer[idx] |= (1 << bit);
    } else {
        oled_buffer[idx] &= ~(1 << bit);
    }
}

void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int16_t dx = abs(x2 - x1);
    int16_t dy = abs(y2 - y1);
    int16_t sx = (x1 < x2) ? 1 : -1;
    int16_t sy = (y1 < y2) ? 1 : -1;
    int16_t err = dx - dy;
    
    while(1) {
        OLED_DrawPixel(x1, y1, true);
        
        if(x1 == x2 && y1 == y2) break;
        
        int16_t e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool filled)
{
    if(filled) {
        for(uint8_t i = 0; i < height; i++) {
            for(uint8_t j = 0; j < width; j++) {
                OLED_DrawPixel(x + j, y + i, true);
            }
        }
    } else {
        OLED_DrawLine(x, y, x + width - 1, y);
        OLED_DrawLine(x, y + height - 1, x + width - 1, y + height - 1);
        OLED_DrawLine(x, y, x, y + height - 1);
        OLED_DrawLine(x + width - 1, y, x + width - 1, y + height - 1);
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, OLED_Font_e font)
{
    if(ch < ' ' || ch > '~') return;  // Only printable ASCII
    if(font != OLED_FONT_6X8) return;  // Only 6x8 font for now
    
    uint8_t char_idx = ch - ' ';
    uint8_t page = y;
    
    if(page >= OLED_PAGES || x >= OLED_WIDTH) return;
    
    for(uint8_t i = 0; i < 6; i++) {
        if(x + i < OLED_WIDTH) {
            oled_buffer[page * OLED_WIDTH + x + i] = font_6x8[char_idx][i];
        }
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str, OLED_Font_e font)
{
    uint8_t pos_x = x;
    
    while(*str != '\0') {
        if(pos_x >= OLED_WIDTH) break;
        OLED_ShowChar(pos_x, y, *str, font);
        pos_x += 6;  // 6x8 font width
        str++;
    }
}

void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, OLED_Font_e font)
{
    char str[12];
    snprintf(str, sizeof(str), "%*d", len, (int)num);
    OLED_ShowString(x, y, str, font);
}

void OLED_ShowLogo(void)
{
    OLED_Clear();
    
    // Simple centered text logo
    OLED_ShowString(16, 2, "Three-Channel", OLED_FONT_6X8);
    OLED_ShowString(28, 4, "Controller", OLED_FONT_6X8);
    OLED_ShowString(40, 6, "v4.0", OLED_FONT_6X8);
    
    // Draw border
    OLED_DrawRect(0, 0, OLED_WIDTH, OLED_HEIGHT, false);
    
    OLED_Refresh();
}

void OLED_ShowProgress(uint8_t percent)
{
    if(percent > 100) percent = 100;
    
    OLED_Clear();
    
    // Title
    OLED_ShowString(30, 1, "Loading...", OLED_FONT_6X8);
    
    // Progress bar frame
    OLED_DrawRect(14, 28, 100, 10, false);
    
    // Progress bar fill
    uint8_t fill_width = (98 * percent) / 100;
    if(fill_width > 0) {
        OLED_DrawRect(15, 29, fill_width, 8, true);
    }
    
    // Percentage text
    char str[8];
    snprintf(str, sizeof(str), "%d%%", percent);
    OLED_ShowString(56, 5, str, OLED_FONT_6X8);
    
    OLED_Refresh();
}

void OLED_Refresh(void)
{
    // Set column address range (0-127)
    oled_write_cmd(0x21);
    oled_write_cmd(0);
    oled_write_cmd(127);
    
    // Set page address range (0-7)
    oled_write_cmd(0x22);
    oled_write_cmd(0);
    oled_write_cmd(7);
    
    // Send entire buffer
    oled_write_data(oled_buffer, sizeof(oled_buffer));
}

void OLED_DisplayOn(bool on)
{
    oled_write_cmd(on ? 0xAF : 0xAE);
}

void OLED_SetBrightness(uint8_t brightness)
{
    oled_write_cmd(0x81);  // Set contrast control
    oled_write_cmd(brightness);
}

void OLED_Test(void)
{
    printf("[OLED] Running display test...\r\n");
    
    // Test 1: Clear screen
    printf("[OLED] Test 1: Clear screen\r\n");
    OLED_Clear();
    OLED_Refresh();
    HAL_Delay(500);
    
    // Test 2: Draw lines
    printf("[OLED] Test 2: Draw lines\r\n");
    OLED_Clear();
    OLED_DrawLine(0, 0, 127, 63);
    OLED_DrawLine(0, 63, 127, 0);
    OLED_DrawRect(10, 10, 108, 44, false);
    OLED_Refresh();
    HAL_Delay(1000);
    
    // Test 3: Show text
    printf("[OLED] Test 3: Show text\r\n");
    OLED_Clear();
    OLED_ShowString(0, 0, "OLED Test OK", OLED_FONT_6X8);
    OLED_ShowString(0, 2, "Resolution:", OLED_FONT_6X8);
    OLED_ShowString(0, 3, "128x64", OLED_FONT_6X8);
    OLED_ShowString(0, 5, "Controller:", OLED_FONT_6X8);
    OLED_ShowString(0, 6, "SSD1309", OLED_FONT_6X8);
    OLED_Refresh();
    HAL_Delay(2000);
    
    // Test 4: Logo
    printf("[OLED] Test 4: Show logo\r\n");
    OLED_ShowLogo();
    HAL_Delay(2000);
    
    // Test 5: Progress bar
    printf("[OLED] Test 5: Progress bar\r\n");
    for(uint8_t i = 0; i <= 100; i += 10) {
        OLED_ShowProgress(i);
        printf("[OLED] Progress: %d%%\r\n", i);
        HAL_Delay(300);
    }
    
    printf("[OLED] Display test complete!\r\n");
}

/* Private functions ---------------------------------------------------------*/

static bool oled_write_cmd(uint8_t cmd)
{
    uint8_t data[2] = {OLED_CMD_MODE, cmd};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, 
                                                        data, 2, OLED_TIMEOUT);
    return (status == HAL_OK);
}

static bool oled_write_data(uint8_t *data, uint16_t len)
{
    // Send data in chunks (I2C buffer limit)
    #define CHUNK_SIZE 32
    uint16_t sent = 0;
    
    while(sent < len) {
        uint16_t chunk = (len - sent) > CHUNK_SIZE ? CHUNK_SIZE : (len - sent);
        uint8_t buffer[CHUNK_SIZE + 1];
        
        buffer[0] = OLED_DATA_MODE;
        memcpy(&buffer[1], &data[sent], chunk);
        
        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS,
                                                            buffer, chunk + 1, OLED_TIMEOUT);
        if(status != HAL_OK) return false;
        
        sent += chunk;
    }
    
    return true;
}

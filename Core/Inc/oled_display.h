/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : oled_display.h
  * @brief          : OLED display driver header (SSD1309)
  * @author         : Three-channel Controller Team
  * @date           : 2026-02-13
  ******************************************************************************
  * @attention
  *
  * OLED Hardware:
  * - Size: 2.42 inch
  * - Resolution: 128x64 pixels
  * - Controller: SSD1309
  * - Interface: I2C (Address: 0x78)
  * - Connection: PB6(SCL), PB7(SDA)
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __OLED_DISPLAY_H
#define __OLED_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief OLED display area definition
 */
typedef enum {
    OLED_AREA_ALARM = 0,    ///< Alarm info area (top, row 0-1)
    OLED_AREA_CHANNEL,      ///< Channel status area (middle, row 2-5)
    OLED_AREA_TEMP,         ///< Temperature/Fan area (bottom, row 6-7)
    OLED_AREA_MAX
} OLED_Area_e;

/**
 * @brief Font size enumeration
 */
typedef enum {
    OLED_FONT_6X8 = 0,      ///< 6x8 ASCII font
    OLED_FONT_8X16,         ///< 8x16 ASCII font
    OLED_FONT_MAX
} OLED_Font_e;

/* Exported constants --------------------------------------------------------*/

/**
 * @brief OLED hardware parameters
 */
#define OLED_I2C_ADDRESS    0x78    ///< SSD1309 I2C address (7-bit: 0x3C, 8-bit: 0x78)
#define OLED_WIDTH          128     ///< Screen width in pixels
#define OLED_HEIGHT         64      ///< Screen height in pixels
#define OLED_PAGES          8       ///< Number of pages (64/8=8)

/**
 * @brief Display area boundaries (in rows, 8 pixels per row)
 */
#define OLED_AREA_ALARM_START   0   ///< Alarm area start row
#define OLED_AREA_ALARM_END     1   ///< Alarm area end row
#define OLED_AREA_CHANNEL_START 2   ///< Channel area start row
#define OLED_AREA_CHANNEL_END   5   ///< Channel area end row
#define OLED_AREA_TEMP_START    6   ///< Temperature area start row
#define OLED_AREA_TEMP_END      7   ///< Temperature area end row

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Initialize OLED display
 * @note   Must be called before any other OLED function
 * @retval true if success, false if failed
 */
bool OLED_Init(void);

/**
 * @brief  Clear entire screen
 * @note   Clears all pixels and refreshes display
 * @retval None
 */
void OLED_Clear(void);

/**
 * @brief  Clear specific area
 * @param  area: Area to clear (OLED_AREA_ALARM/CHANNEL/TEMP)
 * @retval None
 */
void OLED_ClearArea(OLED_Area_e area);

/**
 * @brief  Set a single pixel
 * @param  x: X coordinate (0-127)
 * @param  y: Y coordinate (0-63)
 * @param  on: true=turn on, false=turn off
 * @retval None
 */
void OLED_DrawPixel(uint8_t x, uint8_t y, bool on);

/**
 * @brief  Draw a line
 * @param  x1: Start X coordinate
 * @param  y1: Start Y coordinate
 * @param  x2: End X coordinate
 * @param  y2: End Y coordinate
 * @retval None
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief  Draw a rectangle
 * @param  x: Top-left X coordinate
 * @param  y: Top-left Y coordinate
 * @param  width: Rectangle width
 * @param  height: Rectangle height
 * @param  filled: true=filled, false=outline only
 * @retval None
 */
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool filled);

/**
 * @brief  Show ASCII string
 * @param  x: Start X coordinate (column)
 * @param  y: Start Y coordinate (row, 0-7)
 * @param  str: String to display (ASCII only)
 * @param  font: Font size (OLED_FONT_6X8 or OLED_FONT_8X16)
 * @retval None
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, OLED_Font_e font);

/**
 * @brief  Show single ASCII character
 * @param  x: X coordinate (column)
 * @param  y: Y coordinate (row, 0-7)
 * @param  ch: Character to display
 * @param  font: Font size
 * @retval None
 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, OLED_Font_e font);

/**
 * @brief  Show integer number
 * @param  x: X coordinate
 * @param  y: Y coordinate
 * @param  num: Number to display
 * @param  len: Display length (pad with spaces if needed)
 * @param  font: Font size
 * @retval None
 */
void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, OLED_Font_e font);

/**
 * @brief  Display customer logo (centered, bitmap rendering)
 * @note   Shows logo at startup; uses row-scan bitmap data
 * @retval None
 */
void OLED_ShowLogo(void);

/**
 * @brief  Display Minyer brand logo in self-test progress screen
 * @note   Renders 115x27 px Image2Lcd bitmap into row1~4 (y=10~36)
 * @retval None
 */
void OLED_ShowMinyerLogo(void);

/**
 * @brief  Display progress bar
 * @param  percent: Progress percentage (0-100)
 * @retval None
 */
void OLED_ShowProgress(uint8_t percent);

/**
 * @brief  Refresh display (update screen from buffer)
 * @note   Call this after drawing operations to make changes visible
 * @retval None
 */
void OLED_Refresh(void);

/**
 * @brief  Turn display on/off
 * @param  on: true=display on, false=display off
 * @retval None
 */
void OLED_DisplayOn(bool on);

/**
 * @brief  Set display brightness
 * @param  brightness: Brightness level (0-255)
 * @retval None
 */
void OLED_SetBrightness(uint8_t brightness);

/**
 * @brief  Test OLED display (for debugging)
 * @note   Displays test pattern and information
 * @retval None
 */
void OLED_Test(void);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_DISPLAY_H */

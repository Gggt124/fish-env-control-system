#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Screen dimensions in landscape mode
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// Predefined 16-bit RGB565 color constants (big-endian for SPI transfer)
// ILI9341 driver expects color high byte first in big-endian over SPI
#define TFT_COLOR_BLACK   0x0000
#define TFT_COLOR_WHITE   0xFFFF
#define TFT_COLOR_RED     0xF800
#define TFT_COLOR_GREEN   0x07E0
#define TFT_COLOR_BLUE    0x001F
#define TFT_COLOR_YELLOW  0xFFE0
#define TFT_COLOR_CYAN    0x07FF
#define TFT_COLOR_MAGENTA 0xF81F
#define TFT_COLOR_ORANGE  0xFD20
#define TFT_COLOR_GRAY    0x7BEF
#define TFT_COLOR_DARK_NAVY 0x0821
#define TFT_COLOR_DARK_PANEL 0x18E3

/**
 * @brief Initialize the SPI bus and ILI9341 TFT display using esp_lcd.
 *        Renders the boot splash screen immediately upon success.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t tft_display_init(void);

/**
 * @brief Clear the entire screen to a specified color.
 * @param color RGB565 color.
 */
void tft_clear(uint16_t color);

/**
 * @brief Draw a single character using the embedded 8x16 font.
 * @param x X coordinate (0 to TFT_WIDTH-1)
 * @param y Y coordinate (0 to TFT_HEIGHT-1)
 * @param c The character to render
 * @param color Foreground RGB565 color
 * @param bg_color Background RGB565 color
 */
void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color);

/**
 * @brief Draw a single character scaled by 2 (16x32 pixels) using the embedded 8x16 font.
 * @param x X coordinate
 * @param y Y coordinate
 * @param c The character to render
 * @param color Foreground RGB565 color
 * @param bg_color Background RGB565 color
 */
void tft_draw_char_x2(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color);

/**
 * @brief Draw a string of characters.
 *        Safe bounds-checking will prevent overflow outside the screen.
 * @param x X coordinate
 * @param y Y coordinate
 * @param str The null-terminated string to render
 * @param color Foreground RGB565 color
 * @param bg_color Background RGB565 color
 */
void tft_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color);

/**
 * @brief Draw a string of characters scaled by 2 (16x32 pixels).
 *        Safe bounds-checking will prevent overflow outside the screen.
 * @param x X coordinate
 * @param y Y coordinate
 * @param str The null-terminated string to render
 * @param color Foreground RGB565 color
 * @param bg_color Background RGB565 color
 */
void tft_draw_string_x2(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color);

/**
 * @brief Fill a rectangular area with a specified color.
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width of the rectangle
 * @param h Height of the rectangle
 * @param color RGB565 color
 */
void tft_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draw the static landscape layout skeleton of the status dashboard.
 */
void tft_display_draw_dashboard_skeleton(void);

/**
 * @brief Start the background periodic update FreeRTOS task.
 */
void tft_display_start_task(void);

/**
 * @brief Control TFT backlight on/off
 * @param on true = backlight on, false = backlight off (screen saver)
 */
void tft_display_set_backlight(bool on);

/**
 * @brief Reset the backlight idle timer.
 * Call from BOTH hardware events (pump state change, float switch change, phase
 * transition) AND web API activity. Backlight auto-off after
 * APP_TEMPLATE_SCREEN_TIMEOUT_MS of inactivity from any source.
 *
 * Rationale: TFT is a 24/7 status display (shows IP/mDNS/pump state without
 * browser) — idle must be measured from real device activity, not only web usage.
 */
void tft_display_reset_idle_timer(void);

/** Screen off timeout: 30 min. Long because TFT is primary status display.
 *  Burn-in risk is low: dynamic content (countdown, temp) changes every second.
 *  Only static labels risk burn-in if pump is completely idle for hours. */
#define APP_TEMPLATE_SCREEN_TIMEOUT_MS   (30 * 60 * 1000)

#ifdef __cplusplus
}
#endif

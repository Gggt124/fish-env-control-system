#include "tft_display.h"
#include "font8x16.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "app_config.h"

static const char *TAG = "TFT_DISPLAY";

static esp_lcd_panel_handle_t s_panel_handle = NULL;
static SemaphoreHandle_t s_trans_done_sem = NULL;

// Helper to swap bytes for 16-bit RGB565 to match big-endian SPI transmission
#define SWAP_BYTES(val) ((((val) & 0xff) << 8) | (((val) & 0xff00) >> 8))

static bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    BaseType_t high_task_awoken = pdFALSE;
    if (s_trans_done_sem) {
        xSemaphoreGiveFromISR(s_trans_done_sem, &high_task_awoken);
    }
    return high_task_awoken == pdTRUE;
}

esp_err_t tft_display_init(void) {
    ESP_LOGI(TAG, "Initializing TFT display on VSPI (SPI3_HOST)");

    // 1. Create binary semaphore for DMA transfer synchronization
    s_trans_done_sem = xSemaphoreCreateBinary();
    if (!s_trans_done_sem) {
        ESP_LOGE(TAG, "Failed to create transfer done semaphore");
        return ESP_ERR_NO_MEM;
    }

    // 2. Turn on LED backlight pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << APP_TEMPLATE_TFT_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure backlight GPIO: %s", esp_err_to_name(ret));
        return ret;
    }
    gpio_set_level(APP_TEMPLATE_TFT_LED_GPIO, 1);
    ESP_LOGI(TAG, "Backlight turned ON");

    // 3. Initialize SPI Bus on SPI3_HOST (VSPI)
    spi_bus_config_t buscfg = {
        .sclk_io_num = APP_TEMPLATE_TFT_SCK_GPIO,
        .mosi_io_num = APP_TEMPLATE_TFT_MOSI_GPIO,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_WIDTH * 32 * sizeof(uint16_t),
    };
    ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Configure SPI Panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = APP_TEMPLATE_TFT_DC_GPIO,
        .cs_gpio_num = APP_TEMPLATE_TFT_CS_GPIO,
        .pclk_hz = 40 * 1000 * 1000, // 40MHz
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = on_color_trans_done,
    };
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI3_HOST, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
        return ret;
    }

    // 5. Install ILI9341 panel driver
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = APP_TEMPLATE_TFT_RESET_GPIO,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ret = esp_lcd_new_panel_ili9341(io_handle, &panel_config, &s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ILI9341 panel driver: %s", esp_err_to_name(ret));
        return ret;
    }

    // 6. Reset and initialize display panel hardware
    ret = esp_lcd_panel_reset(s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset panel: %s", esp_err_to_name(ret));
        return ret;
    }
    ret = esp_lcd_panel_init(s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init panel: %s", esp_err_to_name(ret));
        return ret;
    }
    ret = esp_lcd_panel_disp_on_off(s_panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn display ON: %s", esp_err_to_name(ret));
        return ret;
    }

    // 7. Setup screen layout (Landscape 320x240)
    ret = esp_lcd_panel_swap_xy(s_panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to swap XY axes: %s", esp_err_to_name(ret));
        return ret;
    }
    ret = esp_lcd_panel_mirror(s_panel_handle, false, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mirroring: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "TFT display successfully initialized");
    return ESP_OK;
}

void tft_clear(uint16_t color) {
    tft_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || x + 8 > TFT_WIDTH || y + 16 > TFT_HEIGHT) {
        return;
    }

    // 8x16 font has 128 pixels (256 bytes buffer, well under the 1024-byte limit)
    uint16_t pixel_buf[128];
    uint16_t color_be = SWAP_BYTES(color);
    uint16_t bg_color_be = SWAP_BYTES(bg_color);

    const uint8_t *bitmap = font8x16[(uint8_t)c];

    for (int row = 0; row < 16; row++) {
        uint8_t bits = bitmap[row];
        for (int col = 0; col < 8; col++) {
            bool pixel_on = (bits & (0x80 >> col)) != 0;
            pixel_buf[row * 8 + col] = pixel_on ? color_be : bg_color_be;
        }
    }

    esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel_handle, x, y, x + 8, y + 16, pixel_buf);
    if (err == ESP_OK) {
        xSemaphoreTake(s_trans_done_sem, portMAX_DELAY);
    }
}

void tft_draw_char_x2(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || x + 16 > TFT_WIDTH || y + 32 > TFT_HEIGHT) {
        return;
    }

    // 16x32 scaled character has 512 pixels (1024 bytes buffer, exactly matching the limit)
    uint16_t pixel_buf[512];
    uint16_t color_be = SWAP_BYTES(color);
    uint16_t bg_color_be = SWAP_BYTES(bg_color);

    const uint8_t *bitmap = font8x16[(uint8_t)c];

    for (int row = 0; row < 16; row++) {
        uint8_t bits = bitmap[row];
        for (int col = 0; col < 8; col++) {
            bool pixel_on = (bits & (0x80 >> col)) != 0;
            uint16_t p_color = pixel_on ? color_be : bg_color_be;

            int base_row1 = (row * 2) * 16;
            int base_row2 = (row * 2 + 1) * 16;
            int col2 = col * 2;

            pixel_buf[base_row1 + col2] = p_color;
            pixel_buf[base_row1 + col2 + 1] = p_color;
            pixel_buf[base_row2 + col2] = p_color;
            pixel_buf[base_row2 + col2 + 1] = p_color;
        }
    }

    esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel_handle, x, y, x + 16, y + 32, pixel_buf);
    if (err == ESP_OK) {
        xSemaphoreTake(s_trans_done_sem, portMAX_DELAY);
    }
}

void tft_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !str || y >= TFT_HEIGHT) {
        return;
    }
    uint16_t cur_x = x;
    while (*str) {
        if (cur_x + 8 > TFT_WIDTH) {
            break; // bounds check: truncate line if it goes out of screen dimensions
        }
        tft_draw_char(cur_x, y, *str, color, bg_color);
        cur_x += 8;
        str++;
    }
}

void tft_draw_string_x2(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !str || y >= TFT_HEIGHT) {
        return;
    }
    uint16_t cur_x = x;
    while (*str) {
        if (cur_x + 16 > TFT_WIDTH) {
            break; // bounds check: truncate line if it goes out of screen dimensions
        }
        tft_draw_char_x2(cur_x, y, *str, color, bg_color);
        cur_x += 16;
        str++;
    }
}

void tft_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (!s_panel_handle || x >= TFT_WIDTH || y >= TFT_HEIGHT || w == 0 || h == 0) {
        return;
    }
    if (x + w > TFT_WIDTH) {
        w = TFT_WIDTH - x;
    }
    if (y + h > TFT_HEIGHT) {
        h = TFT_HEIGHT - y;
    }

    uint16_t big_endian_color = SWAP_BYTES(color);

    // Limit stack allocation to 320 elements (640 bytes buffer, well under the 1024-byte limit)
    uint16_t chunk_buf[320];
    for (int i = 0; i < w; i++) {
        chunk_buf[i] = big_endian_color;
    }

    for (uint16_t cy = y; cy < y + h; cy++) {
        esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel_handle, x, cy, x + w, cy + 1, chunk_buf);
        if (err == ESP_OK) {
            xSemaphoreTake(s_trans_done_sem, portMAX_DELAY);
        }
    }
}

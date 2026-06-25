#include "tft_display.h"
#include "font8x16.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"


#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "app_config.h"
#include "pump_control.h"
#include "cooling_control.h"
#include "wifi_manager.h"
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "TFT_DISPLAY";

static esp_lcd_panel_handle_t s_panel_handle = NULL;
static SemaphoreHandle_t s_trans_done_sem = NULL;
static SemaphoreHandle_t s_tft_mutex = NULL;
static volatile uint32_t s_last_activity_sec = 0;   /* set at init */

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
    esp_err_t ret = ESP_OK;
    esp_lcd_panel_io_handle_t io_handle = NULL;
    bool spi_initialized = false;

    // 1. Create binary semaphore for DMA transfer synchronization and mutex for thread-safety
    s_trans_done_sem = xSemaphoreCreateBinary();
    if (!s_trans_done_sem) {
        ESP_LOGE(TAG, "Failed to create transfer done semaphore");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }
    s_tft_mutex = xSemaphoreCreateMutex();
    if (!s_tft_mutex) {
        ESP_LOGE(TAG, "Failed to create TFT mutex");
        ret = ESP_ERR_NO_MEM;
        goto err;
    }

    // 2. Turn on LED backlight pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << APP_TEMPLATE_TFT_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure backlight GPIO: %s", esp_err_to_name(ret));
        goto err;
    }
    gpio_set_level(APP_TEMPLATE_TFT_LED_GPIO, 1);
    ESP_LOGI(TAG, "Backlight turned ON");
    s_last_activity_sec = (uint32_t)(esp_timer_get_time() / 1000000ULL);  /* start idle timer from boot */

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
        goto err;
    }
    spi_initialized = true;

    // 4. Configure SPI Panel IO
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
        goto err;
    }

    // 5. Install panel driver
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = APP_TEMPLATE_TFT_RESET_GPIO,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ST7789 panel driver: %s", esp_err_to_name(ret));
        goto err;
    }


    // 6. Reset and initialize display panel hardware
    ret = esp_lcd_panel_reset(s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset panel: %s", esp_err_to_name(ret));
        goto err;
    }
    ret = esp_lcd_panel_init(s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init panel: %s", esp_err_to_name(ret));
        goto err;
    }
    ret = esp_lcd_panel_invert_color(s_panel_handle, false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to invert color: %s", esp_err_to_name(ret));
        goto err;
    }
    ret = esp_lcd_panel_disp_on_off(s_panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn display ON: %s", esp_err_to_name(ret));
        goto err;
    }

    // 7. Setup screen layout (Landscape 320x240)
    ret = esp_lcd_panel_swap_xy(s_panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to swap XY axes: %s", esp_err_to_name(ret));
        goto err;
    }
    ret = esp_lcd_panel_mirror(s_panel_handle, true, false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mirroring: %s", esp_err_to_name(ret));
        goto err;
    }
    ret = esp_lcd_panel_set_gap(s_panel_handle, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set panel gap: %s", esp_err_to_name(ret));
        goto err;
    }
    ESP_LOGI(TAG, "Panel driver: ST7789; layout: swap_xy=true, mirror=(true,false), gap=(0,0)");


    ESP_LOGI(TAG, "TFT display successfully initialized");
    return ESP_OK;

err:
    if (s_panel_handle) {
        esp_lcd_panel_del(s_panel_handle);
        s_panel_handle = NULL;
    }
    if (io_handle) {
        esp_lcd_panel_io_del(io_handle);
        io_handle = NULL;
    }
    if (spi_initialized) {
        spi_bus_free(SPI3_HOST);
    }
    if (s_tft_mutex) {
        vSemaphoreDelete(s_tft_mutex);
        s_tft_mutex = NULL;
    }
    if (s_trans_done_sem) {
        vSemaphoreDelete(s_trans_done_sem);
        s_trans_done_sem = NULL;
    }
    return ret;
}

void tft_clear(uint16_t color) {
    tft_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !s_tft_mutex || x + 8 > TFT_WIDTH || y + 16 > TFT_HEIGHT) {
        return;
    }

    if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(500)) != pdTRUE) {
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
        xSemaphoreTake(s_trans_done_sem, pdMS_TO_TICKS(500));
    }
    
    xSemaphoreGive(s_tft_mutex);
}

void tft_draw_char_x2(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !s_tft_mutex || x + 16 > TFT_WIDTH || y + 32 > TFT_HEIGHT) {
        return;
    }

    if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(500)) != pdTRUE) {
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
        xSemaphoreTake(s_trans_done_sem, pdMS_TO_TICKS(500));
    }
    
    xSemaphoreGive(s_tft_mutex);
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
    if (!s_panel_handle || !s_tft_mutex || x >= TFT_WIDTH || y >= TFT_HEIGHT || w == 0 || h == 0) {
        return;
    }
    if (x + w > TFT_WIDTH) {
        w = TFT_WIDTH - x;
    }
    if (y + h > TFT_HEIGHT) {
        h = TFT_HEIGHT - y;
    }

    if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(500)) != pdTRUE) {
        return;
    }

    uint16_t big_endian_color = SWAP_BYTES(color);

    // Static (file-scope) buffer — safely protected by s_tft_mutex
    static uint16_t chunk_buf[320];
    for (int i = 0; i < w; i++) {
        chunk_buf[i] = big_endian_color;
    }

    for (uint16_t cy = y; cy < y + h; cy++) {
        esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel_handle, x, cy, x + w, cy + 1, chunk_buf);
        if (err == ESP_OK) {
            if (xSemaphoreTake(s_trans_done_sem, pdMS_TO_TICKS(500)) != pdTRUE) {
                break;
            }
        }
    }
    
    xSemaphoreGive(s_tft_mutex);
}

void tft_draw_rect_outline(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (w < 2 || h < 2) return;
    tft_fill_rect(x, y, w, 1, color);
    tft_fill_rect(x, y + h - 1, w, 1, color);
    tft_fill_rect(x, y + 1, 1, h - 2, color);
    tft_fill_rect(x + w - 1, y + 1, 1, h - 2, color);
}

void tft_display_draw_dashboard_skeleton(void) {
    // 1. Initial State: วาดพื้นหลัง UI ทั้งจอเพียงครั้งเดียว
    tft_fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_COLOR_BLACK);
    // วาด Top Bar พื้นหลัง
    tft_fill_rect(0, 0, 320, 25, TFT_COLOR_DARK_PANEL);
    // วาดพื้นหลัง Card ทั้งสอง
    tft_fill_rect(5, 30, 150, 205, TFT_COLOR_DARK_NAVY);
    tft_fill_rect(165, 30, 150, 205, TFT_COLOR_DARK_NAVY);
    // วาดหัวข้อ Cyan
    tft_fill_rect(5, 30, 150, 20, TFT_COLOR_CYAN);
    tft_fill_rect(165, 30, 150, 20, TFT_COLOR_CYAN);
    // วาดข้อความหัวข้อสีดำ
    tft_draw_string(25, 32, "PUMP CHANNEL", TFT_COLOR_BLACK, TFT_COLOR_CYAN);
    tft_draw_string(185, 32, "COOLING CHANNEL", TFT_COLOR_BLACK, TFT_COLOR_CYAN);
    
    // Status bar static labels
    tft_draw_string(4, 5, "fish-pump.local", TFT_COLOR_WHITE, TFT_COLOR_DARK_PANEL);
    
    // Static Labels Left Card
    tft_draw_string(15, 120, "TIMER :", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
    tft_draw_string(15, 145, "PHASE :", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
    tft_draw_string(15, 170, "NEXT  :", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
    // วาดกรอบ Progress Bar ทิ้งไว้
    tft_draw_rect_outline(15, 200, 130, 15, TFT_COLOR_GRAY);
    
    // Static Labels Right Card
    tft_draw_string(175, 120, "MODE  :", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
    tft_draw_string(175, 145, "RELAY :", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
    tft_draw_string(175, 170, "LOCK  :", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
}

static volatile bool s_backlight_on = true;

void tft_display_set_backlight(bool on)
{
    if (s_backlight_on == on) return;
    s_backlight_on = on;
    gpio_set_level(APP_TEMPLATE_TFT_LED_GPIO, on ? 1 : 0);
    ESP_LOGI(TAG, "Backlight %s (screen saver)", on ? "ON" : "OFF");
}

void tft_display_reset_idle_timer(void)
{
    s_last_activity_sec = (uint32_t)(esp_timer_get_time() / 1000000ULL);
    tft_display_set_backlight(true);
}

static void tft_display_task(void *pvParameters) {
    ESP_LOGI(TAG, "TFT display background update task started");
    
    tft_display_draw_dashboard_skeleton();
    
    static struct {
        char wifi[24];
        uint32_t uptime_sec;
        
        bool pump_running;
        pump_control_active_timer_t pump_active_timer;
        pump_control_timer_phase_t pump_phase;
        uint32_t pump_countdown_sec;
        
        bool cooling_temp_valid;
        float cooling_temp;
        bool cooling_relay_energized;
        cooling_control_mode_t cooling_mode;
        bool cooling_lockout_active;
    } s_tft_cache;
    
    bool s_cache_valid = false;
    
    TickType_t last_wakeup_time = xTaskGetTickCount();
    while (1) {
        pump_control_status_t pump = {0};
        pump_control_get_status(&pump);
        
        cooling_control_status_t cooling = {0};
        cooling_control_get_status(&cooling);
        
        // 1. Wi-Fi status
        char wifi_str[32];
        if (wifi_manager_is_sta_connected()) {
            snprintf(wifi_str, sizeof(wifi_str), "STA: %s", wifi_manager_get_sta_ip());
        } else if (wifi_manager_is_ap_enabled()) {
            snprintf(wifi_str, sizeof(wifi_str), "AP: %s", wifi_manager_get_ap_ip());
        } else {
            snprintf(wifi_str, sizeof(wifi_str), "DISCONNECT     ");
        }
        
        char wifi_formatted[24];
        snprintf(wifi_formatted, sizeof(wifi_formatted), "%-19.19s", wifi_str);
        
        if (!s_cache_valid || strcmp(s_tft_cache.wifi, wifi_formatted) != 0) {
            strcpy(s_tft_cache.wifi, wifi_formatted);
            tft_draw_string(160, 5, wifi_formatted, TFT_COLOR_WHITE, TFT_COLOR_DARK_PANEL);
        }
        
        // 3. Pump Status
        bool pump_running = pump.running;
        pump_control_active_timer_t active_timer = pump.active_timer;
        pump_control_timer_phase_t phase = pump.phase;
        
        bool pump_running_changed = (!s_cache_valid || s_tft_cache.pump_running != pump_running);
        bool pump_timer_changed = (!s_cache_valid || s_tft_cache.pump_active_timer != active_timer);
        bool pump_phase_changed = (!s_cache_valid || s_tft_cache.pump_phase != phase);
        
        if (pump_running_changed || pump_timer_changed || pump_phase_changed) {
            s_tft_cache.pump_running = pump_running;
            s_tft_cache.pump_active_timer = active_timer;
            s_tft_cache.pump_phase = phase;
            
            if (!pump_running) {
                tft_draw_string_x2(11, 60, "STOPPED ", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
                tft_draw_string(75, 120, "NONE    ", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
                tft_draw_string(75, 145, "[IDLE] ", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
            } else {
                char big_str[10];
                char t_chr = (active_timer == PUMP_CONTROL_TIMER_1) ? '1' : '2';
                
                if (phase == PUMP_CONTROL_PHASE_ON) {
                    snprintf(big_str, sizeof(big_str), "T%c RUN  ", t_chr);
                    tft_draw_string_x2(11, 60, big_str, TFT_COLOR_GREEN, TFT_COLOR_DARK_NAVY);
                    tft_draw_string(75, 145, "[ ON ] ", TFT_COLOR_GREEN, TFT_COLOR_DARK_NAVY);
                } else {
                    snprintf(big_str, sizeof(big_str), "T%c IDLE ", t_chr);
                    tft_draw_string_x2(11, 60, big_str, TFT_COLOR_ORANGE, TFT_COLOR_DARK_NAVY);
                    tft_draw_string(75, 145, "[ OFF ]", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
                }
                
                char active_timer_formatted[12];
                snprintf(active_timer_formatted, sizeof(active_timer_formatted), "TIMER %c ", t_chr);
                tft_draw_string(75, 120, active_timer_formatted, TFT_COLOR_WHITE, TFT_COLOR_DARK_NAVY);
            }
        }
        
        // 6. Pump Remaining
        uint32_t countdown_sec = pump.countdown_sec;
        char countdown_formatted[16];
        snprintf(countdown_formatted, sizeof(countdown_formatted), "%lu s     ", countdown_sec);
        
        if (!s_cache_valid || s_tft_cache.pump_countdown_sec != countdown_sec || pump_running_changed) {
            s_tft_cache.pump_countdown_sec = countdown_sec;
            tft_draw_string(75, 170, countdown_formatted, pump_running ? TFT_COLOR_WHITE : TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
            
            if (pump_running && pump.total_duration_sec > 0) {
                float progress = (float)(pump.total_duration_sec - countdown_sec) / pump.total_duration_sec;
                if (progress < 0) progress = 0;
                if (progress > 1) progress = 1;
                uint16_t bar_width = (uint16_t)(126 * progress);
                if (bar_width > 0) {
                    tft_fill_rect(17, 202, bar_width, 11, TFT_COLOR_CYAN);
                }
                if (bar_width < 126) {
                    tft_fill_rect(17 + bar_width, 202, 126 - bar_width, 11, TFT_COLOR_DARK_NAVY);
                }
            } else {
                tft_fill_rect(17, 202, 126, 11, TFT_COLOR_DARK_NAVY);
            }
        }
        
        // 7. Cooling Temp
        bool temp_valid = cooling.temperature_valid;
        float temp_val = cooling.temperature_c;
        float threshold = cooling.threshold_c_x10 / 10.0f;
        float hysteresis = cooling.hysteresis_c_x10 / 10.0f;
        char temp_formatted[12];
        if (temp_valid) {
            snprintf(temp_formatted, sizeof(temp_formatted), "%5.1f C", temp_val);
        } else {
            snprintf(temp_formatted, sizeof(temp_formatted), "%-7s", "  ERR  ");
        }
        
        uint16_t temp_color = TFT_COLOR_RED;
        if (temp_valid) {
            if (temp_val < (threshold - hysteresis)) temp_color = TFT_COLOR_GREEN;
            else if (temp_val < threshold) temp_color = TFT_COLOR_ORANGE;
            else temp_color = TFT_COLOR_RED;
        }
        
        bool temp_changed = !s_cache_valid || s_tft_cache.cooling_temp_valid != temp_valid || 
                             (temp_valid && (s_tft_cache.cooling_temp != temp_val));
        
        if (temp_changed) {
            s_tft_cache.cooling_temp_valid = temp_valid;
            s_tft_cache.cooling_temp = temp_val;
            tft_draw_string_x2(185, 60, temp_formatted, temp_color, TFT_COLOR_DARK_NAVY);
        }
        
        // 8. Cooling Mode
        cooling_control_mode_t cooling_mode = cooling.mode;
        char cooling_mode_formatted[12];
        const char *mode_str = "FORCE_OFF";
        uint16_t mode_color = TFT_COLOR_GRAY;
        if (cooling_mode == COOLING_CONTROL_MODE_AUTO) {
            mode_str = "AUTO";
            mode_color = TFT_COLOR_WHITE;
        } else if (cooling_mode == COOLING_CONTROL_MODE_TEST_ON) {
            mode_str = "TEST_ON";
            mode_color = TFT_COLOR_YELLOW;
        }
        snprintf(cooling_mode_formatted, sizeof(cooling_mode_formatted), "%-10s", mode_str);
        
        if (!s_cache_valid || s_tft_cache.cooling_mode != cooling_mode) {
            s_tft_cache.cooling_mode = cooling_mode;
            tft_draw_string(235, 120, cooling_mode_formatted, mode_color, TFT_COLOR_DARK_NAVY);
        }
        
        // 9. Cooling Relay
        bool cooling_relay = cooling.relay_energized;
        
        if (!s_cache_valid || s_tft_cache.cooling_relay_energized != cooling_relay) {
            s_tft_cache.cooling_relay_energized = cooling_relay;
            if (cooling_relay) {
                tft_draw_string(235, 145, "[ ON ] ", TFT_COLOR_GREEN, TFT_COLOR_DARK_NAVY);
            } else {
                tft_draw_string(235, 145, "[ OFF ]", TFT_COLOR_GRAY, TFT_COLOR_DARK_NAVY);
            }
        }
        
        // 10. Cooling Lockout
        bool lockout_active = cooling.lockout_active;
        char lockout_formatted[12];
        snprintf(lockout_formatted, sizeof(lockout_formatted), "%-10s", lockout_active ? "ACTIVE" : "INACTIVE");
        uint16_t lockout_color = lockout_active ? TFT_COLOR_YELLOW : TFT_COLOR_GRAY;
        
        if (!s_cache_valid || s_tft_cache.cooling_lockout_active != lockout_active) {
            s_tft_cache.cooling_lockout_active = lockout_active;
            tft_draw_string(235, 170, lockout_formatted, lockout_color, TFT_COLOR_DARK_NAVY);
        }
        
        static bool s_prev_relay_energized = false;
        static pump_control_float_state_t s_prev_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;

        if (s_cache_valid) {
            if (pump_running_changed || pump_timer_changed || pump_phase_changed ||
                s_prev_relay_energized != pump.relay_energized ||
                s_prev_float_state != pump.float_state) {
                tft_display_reset_idle_timer();
            }
        }
        s_prev_relay_energized = pump.relay_energized;
        s_prev_float_state = pump.float_state;

        /* A9 audit: auto-off backlight after 30 min inactivity (burn-in prevention).
         * No touch screen — activity is signaled via web API calls instead.
         * Only turn off if pump is completely stopped to match UI behavior. */
        uint32_t now_sec = (uint32_t)(esp_timer_get_time() / 1000000ULL);
        uint32_t idle_ms = (now_sec - s_last_activity_sec) * 1000;
        if (!pump_running && idle_ms > APP_TEMPLATE_SCREEN_TIMEOUT_MS) {
            tft_display_set_backlight(false);
        }

        s_cache_valid = true;
        
        vTaskDelayUntil(&last_wakeup_time, pdMS_TO_TICKS(200));
    }
}
void tft_display_start_task(void) {
    ESP_LOGI(TAG, "Creating TFT display background update task (stack=4096, prio=4)");
    BaseType_t created = xTaskCreate(tft_display_task, "tft_display_task", 4096, NULL, 4, NULL);
    if (created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create tft_display_task: ret=%d", (int)created);
    } else {
        ESP_LOGI(TAG, "tft_display_task created successfully");
    }
}

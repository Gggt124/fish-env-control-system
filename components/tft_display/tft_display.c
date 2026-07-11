#include "tft_display.h"
#include "font8x16.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include <stdatomic.h>

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

/* SPI host selection: S3-WROOM-1 Octal PSRAM occupies SPI3_HOST pins internally.
 * Use SPI2_HOST (FSPI) on S3; keep SPI3_HOST (VSPI) on classic ESP32. */
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define TFT_SPI_HOST  SPI2_HOST
#else
#define TFT_SPI_HOST  SPI3_HOST
#endif

static const char *TAG = "TFT_DISPLAY";

static esp_lcd_panel_handle_t s_panel_handle = NULL;
static SemaphoreHandle_t s_trans_done_sem = NULL;
static SemaphoreHandle_t s_tft_mutex = NULL;
static atomic_uint_fast32_t s_last_activity_sec = ATOMIC_VAR_INIT(0);   /* set at init */
static atomic_bool s_resync_requested = ATOMIC_VAR_INIT(false);

void tft_display_request_resync(void) {
    atomic_store(&s_resync_requested, true);
}

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
    ESP_LOGI(TAG, "Initializing TFT display on SPI host %d", (int)TFT_SPI_HOST);
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

    // 2. Configure LEDC PWM for backlight dimming
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = 1000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        goto err;
    }

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = APP_TEMPLATE_TFT_LED_GPIO,
        .duty           = 255, // 100% on boot
        .hpoint         = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        goto err;
    }
    ESP_LOGI(TAG, "Backlight PWM configured at 100%%");
    atomic_store(&s_last_activity_sec, (uint32_t)(esp_timer_get_time() / 1000000ULL));  /* start idle timer from boot */

    // 3. Initialize SPI Bus (SPI2_HOST/FSPI on S3, SPI3_HOST/VSPI on classic ESP32)
    spi_bus_config_t buscfg = {
        .sclk_io_num = APP_TEMPLATE_TFT_SCK_GPIO,
        .mosi_io_num = APP_TEMPLATE_TFT_MOSI_GPIO,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_WIDTH * 32 * sizeof(uint16_t),
    };
    ret = spi_bus_initialize(TFT_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        goto err;
    }
    spi_initialized = true;

    // 4. Configure SPI Panel IO
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = APP_TEMPLATE_TFT_DC_GPIO,
        .cs_gpio_num = APP_TEMPLATE_TFT_CS_GPIO,
        .pclk_hz = APP_TEMPLATE_TFT_SPI_CLK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = on_color_trans_done,
    };
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TFT_SPI_HOST, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
        goto err;
    }
    /* Bias the DC line HIGH between transactions. The esp_lcd SPI driver
     * tri-states DC (gpio_ll_output_disable) after each transfer, leaving
     * it floating and EMI-vulnerable (no external pull-up on this board).
     * Enabling the internal pull-up keeps DC at data-mode level during the
     * gaps, so a glitch must overcome the pull-up to be read as a command.
     * NOTE: internal ~45k pull-up is a partial mitigation; for production
     * add an external 10k pull-up to 3V3 on APP_TEMPLATE_TFT_DC_GPIO. */
    gpio_set_pull_mode(APP_TEMPLATE_TFT_DC_GPIO, GPIO_PULLUP_ONLY);
    ESP_LOGI(TAG, "DC GPIO %d internal pull-up enabled (data-mode bias between transactions)",
             (int)APP_TEMPLATE_TFT_DC_GPIO);

    // 5. Install panel driver
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = APP_TEMPLATE_TFT_RESET_GPIO,
        .rgb_ele_order = APP_TEMPLATE_TFT_RGB_ELE_ORDER,
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
    ret = esp_lcd_panel_invert_color(s_panel_handle, APP_TEMPLATE_TFT_INVERT_COLOR);
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
    ESP_LOGI(TAG, "Panel driver: ST7789 | rgb_order=%d invert=%d clk=%ldHz | swap_xy=true mirror=(true,false)",
             (int)APP_TEMPLATE_TFT_RGB_ELE_ORDER,
             (int)APP_TEMPLATE_TFT_INVERT_COLOR,
             (long)APP_TEMPLATE_TFT_SPI_CLK_HZ);


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
        spi_bus_free(TFT_SPI_HOST);
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

static void tft_draw_char_locked(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
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
}

static void tft_draw_char_x2_locked(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
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
}

void tft_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !s_tft_mutex || x + 8 > TFT_WIDTH || y + 16 > TFT_HEIGHT) return;
    if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        tft_draw_char_locked(x, y, c, color, bg_color);
        xSemaphoreGive(s_tft_mutex);
    }
}

void tft_draw_char_x2(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !s_tft_mutex || x + 16 > TFT_WIDTH || y + 32 > TFT_HEIGHT) return;
    if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        tft_draw_char_x2_locked(x, y, c, color, bg_color);
        xSemaphoreGive(s_tft_mutex);
    }
}

void tft_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !s_tft_mutex || !str || y >= TFT_HEIGHT) return;
    if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        uint16_t cur_x = x;
        while (*str) {
            if (cur_x + 8 > TFT_WIDTH) break;
            tft_draw_char_locked(cur_x, y, *str, color, bg_color);
            cur_x += 8;
            str++;
        }
        xSemaphoreGive(s_tft_mutex);
    }
}

void tft_draw_string_x2(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color) {
    if (!s_panel_handle || !s_tft_mutex || !str || y >= TFT_HEIGHT) return;
    if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        uint16_t cur_x = x;
        while (*str) {
            if (cur_x + 16 > TFT_WIDTH) break;
            tft_draw_char_x2_locked(cur_x, y, *str, color, bg_color);
            cur_x += 16;
            str++;
        }
        xSemaphoreGive(s_tft_mutex);
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

    // Static (file-scope) buffer — safely protected by s_tft_mutex
    static uint16_t chunk_buf[320];
    uint16_t color_be = SWAP_BYTES(color);
    for (int i = 0; i < w; i++) {
        chunk_buf[i] = color_be;
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
    tft_fill_rect(0, 0, TFT_WIDTH, 25, TFT_COLOR_DARK_PANEL);
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
    tft_draw_string(4, 5, APP_TEMPLATE_MDNS_HOSTNAME ".local", TFT_COLOR_WHITE, TFT_COLOR_DARK_PANEL);
    
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

static atomic_uchar s_current_brightness = ATOMIC_VAR_INIT(100);
static uint8_t s_idle_dim_percent = APP_TEMPLATE_TFT_DIM_PERCENT;

void tft_display_set_idle_dim_percent(uint8_t percent) {
    s_idle_dim_percent = (percent <= 100) ? percent : APP_TEMPLATE_TFT_DIM_PERCENT;
}

uint8_t tft_display_get_idle_dim_percent(void) {
    return s_idle_dim_percent;
}

void tft_display_set_brightness(uint8_t percent)
{
    if (percent > 100) percent = 100;
    if (atomic_load(&s_current_brightness) == percent) return;
    
    atomic_store(&s_current_brightness, percent);
    uint32_t duty = (255 * percent) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "Backlight set to %d%%", percent);
}

void tft_display_reset_idle_timer(void)
{
    atomic_store(&s_last_activity_sec, (uint32_t)(esp_timer_get_time() / 1000000ULL));
    tft_display_set_brightness(100);
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
        bool pump_relay_energized;
        
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
                int32_t elapsed_sec = (int32_t)pump.total_duration_sec - (int32_t)countdown_sec;
                float progress = (elapsed_sec <= 0) ? 0.0f
                               : (elapsed_sec >= (int32_t)pump.total_duration_sec) ? 1.0f
                               : (float)elapsed_sec / (float)pump.total_duration_sec;
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
        bool cooling_relay_edge = (s_cache_valid && s_tft_cache.cooling_relay_energized != cooling_relay);
        
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
        
        /* A9 audit: auto-dim backlight after 5 min inactivity (burn-in prevention).
         * No touch screen — activity is signaled via web API calls or buttons. */
        uint32_t now_sec = (uint32_t)(esp_timer_get_time() / 1000000ULL);
        uint32_t idle_ms = (now_sec - (uint32_t)atomic_load(&s_last_activity_sec)) * 1000;
        if (idle_ms > APP_TEMPLATE_TFT_DIM_TIMEOUT_MS && atomic_load(&s_current_brightness) != s_idle_dim_percent) {
            tft_display_set_brightness(s_idle_dim_percent);
        }

        /* ── Panel register re-sync ──────────────────────────────
         * Trigger: (1) relay state change detected (EMI source)
         *          (2) external request via tft_display_request_resync()
         *          (3) periodic safety net every ~30 s
         * Sends 3 cheap SPI commands to force-correct inversion,
         * orientation, and mirroring registers.  Self-heals any
         * glitch-induced corruption within one task period (200ms). */
        bool pump_relay_now = pump.running &&
                              (pump.phase == PUMP_CONTROL_PHASE_ON);

        bool pump_relay_edge = (s_cache_valid && s_tft_cache.pump_relay_energized != pump_relay_now);
        bool relay_changed = pump_relay_edge || cooling_relay_edge;

        bool ext_request = atomic_exchange(&s_resync_requested, false);

        static uint32_t resync_counter = 0;
        bool periodic = (++resync_counter >= 150);   /* 150 × 200ms = 30 s */

        if (relay_changed || ext_request || periodic) {
            if (xSemaphoreTake(s_tft_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                resync_counter = 0;
                esp_lcd_panel_invert_color(s_panel_handle, APP_TEMPLATE_TFT_INVERT_COLOR);
                esp_lcd_panel_swap_xy(s_panel_handle, true);
                esp_lcd_panel_mirror(s_panel_handle, true, false);
                xSemaphoreGive(s_tft_mutex);
            } else {
                // If mutex is busy, preserve the trigger for the next tick
                if (relay_changed || ext_request) {
                    tft_display_request_resync();
                }
            }
        }

        s_tft_cache.pump_relay_energized = pump_relay_now;

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

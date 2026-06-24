#include "app_config.h"
#include "cooling_control.h"
#include "hardware_map.h"
#include "nvs_store.h"
#include "pump_control.h"
#include "wifi_manager.h"
#include "session.h"
#include "web_server.h"
#include "dns_server.h"
#include "tft_display.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include <stdatomic.h>

static const char *TAG = "app_main";

atomic_bool s_trigger_rollback = ATOMIC_VAR_INIT(false);
atomic_bool g_cancel_rollback_timer = ATOMIC_VAR_INIT(false);
#define s_cancel_rollback g_cancel_rollback_timer

static volatile bool s_trigger_factory_reset = false;

static esp_timer_handle_t s_confirm_timer = NULL;
static esp_timer_handle_t s_wifi_timer = NULL;

static void confirm_timer_cb(void *arg)
{
    (void)arg;
    s_trigger_rollback = true;
    ESP_LOGW(TAG, "Rollback confirm timer expired!");
}

static void wifi_timer_cb(void *arg)
{
    (void)arg;
    if (!wifi_manager_is_sta_connected()) {
        s_trigger_rollback = true;
        ESP_LOGW(TAG, "Wi-Fi connection staging timeout! Triggering rollback.");
    }
}

static bool s_http_server_retry = false;
static int64_t s_last_board_diag_us = 0;
static uint32_t s_last_idle_runtime[portNUM_PROCESSORS] = {0};

static const char *reset_reason_name(esp_reset_reason_t reason)
{
    switch (reason) {
    case ESP_RST_POWERON:
        return "power_on";
    case ESP_RST_EXT:
        return "external_reset";
    case ESP_RST_SW:
        return "software_reset";
    case ESP_RST_PANIC:
        return "panic";
    case ESP_RST_INT_WDT:
        return "interrupt_watchdog";
    case ESP_RST_TASK_WDT:
        return "task_watchdog";
    case ESP_RST_WDT:
        return "other_watchdog";
    case ESP_RST_BROWNOUT:
        return "brownout";
    default:
        return "other";
    }
}

static const char *pump_settings_load_status_name(nvs_store_pump_settings_load_status_t status)
{
    switch (status) {
    case NVS_STORE_PUMP_SETTINGS_LOADED:
        return "saved";
    case NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING:
        return "defaults-missing";
    case NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID:
        return "defaults-invalid";
    case NVS_STORE_PUMP_SETTINGS_DEFAULTS_ERROR:
    default:
        return "defaults-error";
    }
}

static const char *hardware_map_load_status_name(nvs_store_hardware_map_load_status_t status)
{
    switch (status) {
    case NVS_STORE_HARDWARE_MAP_LOADED:
        return "saved";
    case NVS_STORE_HARDWARE_MAP_DEFAULTS_MISSING:
        return "defaults-missing";
    case NVS_STORE_HARDWARE_MAP_DEFAULTS_INVALID:
        return "defaults-invalid";
    case NVS_STORE_HARDWARE_MAP_DEFAULTS_ERROR:
    default:
        return "defaults-error";
    }
}

static const char *pending_hardware_map_load_status_name(nvs_store_pending_hardware_map_load_status_t status)
{
    switch (status) {
    case NVS_STORE_PENDING_HARDWARE_MAP_LOADED:
        return "saved";
    case NVS_STORE_PENDING_HARDWARE_MAP_NOT_FOUND:
        return "not-found";
    case NVS_STORE_PENDING_HARDWARE_MAP_INVALID:
        return "invalid";
    case NVS_STORE_PENDING_HARDWARE_MAP_ERROR:
    default:
        return "error";
    }
}

static const char *cooling_settings_load_status_name(nvs_store_cooling_settings_load_status_t status)
{
    switch (status) {
    case NVS_STORE_COOLING_SETTINGS_LOADED:
        return "saved";
    case NVS_STORE_COOLING_SETTINGS_DEFAULTS_MISSING:
        return "defaults-missing";
    case NVS_STORE_COOLING_SETTINGS_DEFAULTS_INVALID:
        return "defaults-invalid";
    case NVS_STORE_COOLING_SETTINGS_DEFAULTS_ERROR:
    default:
        return "defaults-error";
    }
}

static const char *pump_float_state_name(pump_control_float_state_t state)
{
    switch (state) {
    case PUMP_CONTROL_FLOAT_OFF:
        return "off";
    case PUMP_CONTROL_FLOAT_ON:
        return "on";
    case PUMP_CONTROL_FLOAT_UNKNOWN:
    default:
        return "unknown";
    }
}

static const char *pump_active_timer_name(pump_control_active_timer_t timer)
{
    switch (timer) {
    case PUMP_CONTROL_TIMER_1:
        return "timer1";
    case PUMP_CONTROL_TIMER_2:
        return "timer2";
    case PUMP_CONTROL_TIMER_NONE:
    default:
        return "none";
    }
}

static const char *pump_active_relay_name(pump_control_active_relay_t relay)
{
    switch (relay) {
    case PUMP_CONTROL_RELAY_1:
        return "relay1";
    case PUMP_CONTROL_RELAY_2:
        return "relay2";
    case PUMP_CONTROL_RELAY_NONE:
    default:
        return "none";
    }
}

static const char *pump_phase_name(pump_control_timer_phase_t phase)
{
    switch (phase) {
    case PUMP_CONTROL_PHASE_ON:
        return "on";
    case PUMP_CONTROL_PHASE_OFF:
        return "off";
    case PUMP_CONTROL_PHASE_IDLE:
    default:
        return "idle";
    }
}

static const char *cooling_mode_name(cooling_control_mode_t mode)
{
    switch (mode) {
    case COOLING_CONTROL_MODE_AUTO:
        return "auto";
    case COOLING_CONTROL_MODE_TEST_ON:
        return "test_on";
    case COOLING_CONTROL_MODE_FORCE_OFF:
    default:
        return "force_off";
    }
}

static const char *cooling_sensor_state_name(cooling_control_sensor_state_t state)
{
    switch (state) {
    case COOLING_CONTROL_SENSOR_OK:
        return "ok";
    case COOLING_CONTROL_SENSOR_FAULT:
        return "fault";
    case COOLING_CONTROL_SENSOR_UNKNOWN:
    default:
        return "unknown";
    }
}

static const char *cooling_fault_name(cooling_control_fault_code_t fault)
{
    switch (fault) {
    case COOLING_CONTROL_FAULT_READ_FAILED:
        return "read_failed";
    case COOLING_CONTROL_FAULT_OUT_OF_RANGE:
        return "out_of_range";
    case COOLING_CONTROL_FAULT_CONFIG_INVALID:
        return "config_invalid";
    case COOLING_CONTROL_FAULT_NONE:
    default:
        return "none";
    }
}

static const char *cooling_blocked_reason_name(cooling_control_blocked_reason_t reason)
{
    switch (reason) {
    case COOLING_CONTROL_BLOCKED_COMPRESSOR_LOCKOUT:
        return "compressor_lockout";
    case COOLING_CONTROL_BLOCKED_SENSOR_FAULT:
        return "sensor_fault";
    case COOLING_CONTROL_BLOCKED_FORCE_OFF:
        return "force_off";
    case COOLING_CONTROL_BLOCKED_CONFIG_INVALID:
        return "config_invalid";
    case COOLING_CONTROL_BLOCKED_NONE:
    default:
        return "none";
    }
}

static uint32_t cpu_load_x10(uint32_t idle_delta, uint32_t elapsed_us)
{
    if (elapsed_us == 0 || idle_delta >= elapsed_us) {
        return 0;
    }
    return 1000U - (uint32_t)(((uint64_t)idle_delta * 1000U) / elapsed_us);
}

static void log_board_and_hardware_diagnostics(void)
{
    int64_t now_us = esp_timer_get_time();
    uint32_t idle_runtime[portNUM_PROCESSORS] = {0};
    uint32_t core_load_x10[portNUM_PROCESSORS] = {0};
    uint32_t elapsed_us = 0;
    bool cpu_sample_valid = s_last_board_diag_us != 0;
    uint32_t total_load_x10 = 0;

    for (int core = 0; core < portNUM_PROCESSORS; core++) {
        idle_runtime[core] = (uint32_t)ulTaskGetIdleRunTimeCounterForCore(core);
    }

    if (cpu_sample_valid) {
        elapsed_us = (uint32_t)(now_us - s_last_board_diag_us);
        for (int core = 0; core < portNUM_PROCESSORS; core++) {
            uint32_t idle_delta = idle_runtime[core] - s_last_idle_runtime[core];
            core_load_x10[core] = cpu_load_x10(idle_delta, elapsed_us);
            total_load_x10 += core_load_x10[core];
        }
        total_load_x10 /= portNUM_PROCESSORS;
    }

    s_last_board_diag_us = now_us;
    for (int core = 0; core < portNUM_PROCESSORS; core++) {
        s_last_idle_runtime[core] = idle_runtime[core];
    }

    multi_heap_info_t heap_info = {0};
    heap_caps_get_info(&heap_info, MALLOC_CAP_INTERNAL);
    esp_err_t twdt_status = esp_task_wdt_status(NULL);

    ESP_LOGI(TAG,
             "[BOARD_DIAG] uptime_ms=%llu cpu_mhz=%d cpu_sample=%s cpu_window_ms=%lu cpu_load_pct=%lu.%lu core0_pct=%lu.%lu core1_pct=%lu.%lu tasks=%lu twdt_main=%s reset=%s heap_free=%lu heap_min=%lu heap_largest=%lu heap_alloc_blocks=%lu heap_free_blocks=%lu main_stack_hwm=%lu die_temp=unsupported external_temp_source=ds18b20",
             (unsigned long long)(now_us / 1000),
             CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
             cpu_sample_valid ? "valid" : "warming_up",
             (unsigned long)(elapsed_us / 1000),
             (unsigned long)(total_load_x10 / 10),
             (unsigned long)(total_load_x10 % 10),
             (unsigned long)(core_load_x10[0] / 10),
             (unsigned long)(core_load_x10[0] % 10),
             (unsigned long)(core_load_x10[1] / 10),
             (unsigned long)(core_load_x10[1] % 10),
             (unsigned long)uxTaskGetNumberOfTasks(),
             esp_err_to_name(twdt_status),
             reset_reason_name(esp_reset_reason()),
             (unsigned long)heap_info.total_free_bytes,
             (unsigned long)heap_info.minimum_free_bytes,
             (unsigned long)heap_info.largest_free_block,
             (unsigned long)heap_info.allocated_blocks,
             (unsigned long)heap_info.free_blocks,
             (unsigned long)uxTaskGetStackHighWaterMark(NULL));

    pump_control_status_t pump = {0};
    if (pump_control_get_status(&pump)) {
        ESP_LOGI(TAG,
                 "[PUMP_DIAG] init=%s config=%s running=%s stabilizing=%s fault=%s float_gpio=%d float=%s active_timer=%s active_relay=%s phase=%s countdown_sec=%lu relay1_gpio=%d relay1=%s relay2_gpio=%d relay2=%s",
                 pump.initialized ? "true" : "false",
                 pump.config_valid ? "true" : "false",
                 pump.running ? "true" : "false",
                 pump.initial_stabilizing ? "true" : "false",
                 pump.fault ? "true" : "false",
                 pump.float_gpio,
                 pump_float_state_name(pump.float_state),
                 pump_active_timer_name(pump.active_timer),
                 pump_active_relay_name(pump.active_relay),
                 pump_phase_name(pump.phase),
                 (unsigned long)pump.countdown_sec,
                 pump.relay1_gpio,
                 pump.relay1_energized ? "on" : "off",
                 pump.relay2_gpio,
                 pump.relay2_energized ? "on" : "off");
    } else {
        ESP_LOGW(TAG, "[PUMP_DIAG] status=unavailable");
    }

    cooling_control_status_t cooling = {0};
    if (cooling_control_get_status(&cooling)) {
        ESP_LOGI(TAG,
                 "[COOLING_DIAG] init=%s config=%s fault=%s sensor=%s fault_code=%s temp_valid=%s temp_c=%.2f ds18b20_gpio=%d mode=%s auto=%s demand=%s relay_gpio=%d relay=%s blocked=%s lockout=%s lockout_sec=%lu test_sec=%lu threshold_x10=%ld hysteresis_x10=%ld",
                 cooling.initialized ? "true" : "false",
                 cooling.config_valid ? "true" : "false",
                 cooling.fault ? "true" : "false",
                 cooling_sensor_state_name(cooling.sensor_state),
                 cooling_fault_name(cooling.fault_code),
                 cooling.temperature_valid ? "true" : "false",
                 cooling.temperature_c,
                 cooling.ds18b20_gpio,
                 cooling_mode_name(cooling.mode),
                 cooling.auto_enable ? "true" : "false",
                 cooling.cooling_demand ? "true" : "false",
                 cooling.cooling_relay_gpio,
                 cooling.relay_energized ? "on" : "off",
                 cooling_blocked_reason_name(cooling.blocked_reason),
                 cooling.lockout_active ? "true" : "false",
                 (unsigned long)cooling.lockout_remaining_sec,
                 (unsigned long)cooling.test_remaining_sec,
                 (long)cooling.threshold_c_x10,
                 (long)cooling.hysteresis_c_x10);
    } else {
        ESP_LOGW(TAG, "[COOLING_DIAG] status=unavailable");
    }
}

static pump_control_relay_polarity_t pump_relay_polarity_from_bool(bool active_low)
{
    return active_low ? PUMP_CONTROL_RELAY_ACTIVE_LOW : PUMP_CONTROL_RELAY_ACTIVE_HIGH;
}

static cooling_control_relay_polarity_t cooling_relay_polarity_from_hardware(
    hardware_relay_polarity_t polarity)
{
    return polarity == HARDWARE_RELAY_ACTIVE_LOW
        ? COOLING_CONTROL_RELAY_ACTIVE_LOW
        : COOLING_CONTROL_RELAY_ACTIVE_HIGH;
}

static cooling_control_mode_t cooling_mode_from_hardware(hardware_cooling_mode_t mode)
{
    switch (mode) {
    case HARDWARE_COOLING_MODE_AUTO:
        return COOLING_CONTROL_MODE_AUTO;
    case HARDWARE_COOLING_MODE_TEST_ON:
        return COOLING_CONTROL_MODE_TEST_ON;
    case HARDWARE_COOLING_MODE_FORCE_OFF:
    default:
        return COOLING_CONTROL_MODE_FORCE_OFF;
    }
}

static pump_control_start_phase_t pump_start_phase_from_hardware(hardware_timer_start_phase_t phase)
{
    return phase == HARDWARE_TIMER_START_PHASE_OFF
        ? PUMP_CONTROL_START_PHASE_OFF
        : PUMP_CONTROL_START_PHASE_ON;
}

static bool promote_pending_hardware_map_on_boot(
    hardware_map_t *active_hardware_map,
    nvs_store_hardware_map_load_status_t *hardware_map_status,
    bool *promoted)
{
    if (promoted) {
        *promoted = false;
    }
    if (!active_hardware_map || !hardware_map_status) {
        return false;
    }

    hardware_map_t pending_hardware_map;
    nvs_store_pending_hardware_map_load_status_t pending_status =
        nvs_store_load_pending_hardware_map(&pending_hardware_map);
    if (pending_status == NVS_STORE_PENDING_HARDWARE_MAP_NOT_FOUND) {
        return true;
    }
    if (pending_status != NVS_STORE_PENDING_HARDWARE_MAP_LOADED) {
        ESP_LOGW(TAG, "Pending hardware map %s; keeping active map",
                 pending_hardware_map_load_status_name(pending_status));
        return false;
    }
    if (hardware_map_equal(active_hardware_map, &pending_hardware_map)) {
        if (!nvs_store_clear_pending_hardware_map()) {
            ESP_LOGE(TAG, "Pending hardware map already active, but clear failed");
            return false;
        }
        ESP_LOGI(TAG, "Pending hardware map already active; cleared pending state");
        return true;
    }
    if (!nvs_store_save_hardware_map(&pending_hardware_map)) {
        ESP_LOGE(TAG, "Failed to promote pending hardware map to active map");
        return false;
    }
    if (!nvs_store_clear_pending_hardware_map()) {
        *active_hardware_map = pending_hardware_map;
        *hardware_map_status = NVS_STORE_HARDWARE_MAP_LOADED;
        if (promoted) {
            *promoted = true;
        }
        ESP_LOGE(TAG, "Pending hardware map promoted, but pending clear failed");
        return false;
    }

    *active_hardware_map = pending_hardware_map;
    *hardware_map_status = NVS_STORE_HARDWARE_MAP_LOADED;
    if (promoted) {
        *promoted = true;
    }
    ESP_LOGI(TAG, "Pending hardware map promoted to active map on boot");
    return true;
}

static void apply_pump_settings_to_config(pump_control_config_t *config,
                                          const nvs_store_pump_settings_t *settings,
                                          const hardware_map_t *hardware_map)
{
    if (!config || !settings || !hardware_map) {
        return;
    }

    config->float_gpio = hardware_map->float_input_gpio;
    config->relay_gpio = hardware_map->pump_relay1_gpio;
    config->relay1_gpio = hardware_map->pump_relay1_gpio;
    config->relay2_gpio = hardware_map->pump_relay2_gpio;
    config->timer1.on_sec = settings->timer1_on_sec;
    config->timer1.off_sec = settings->timer1_off_sec;
    config->timer2.on_sec = settings->timer2_on_sec;
    config->timer2.off_sec = settings->timer2_off_sec;
    config->relay_polarity = pump_relay_polarity_from_bool(settings->relay1_active_low);
    config->relay1_polarity = pump_relay_polarity_from_bool(settings->relay1_active_low);
    config->relay2_polarity = pump_relay_polarity_from_bool(settings->relay2_active_low);
    config->timer1_start_phase = pump_start_phase_from_hardware(settings->timer1_start_phase);
    config->timer2_start_phase = pump_start_phase_from_hardware(settings->timer2_start_phase);
}

static void apply_cooling_settings_to_config(cooling_control_config_t *config,
                                             const nvs_store_cooling_settings_t *settings,
                                             const hardware_map_t *hardware_map)
{
    if (!config || !settings || !hardware_map) {
        return;
    }

    config->ds18b20_gpio = hardware_map->ds18b20_data_gpio;
    config->cooling_relay_gpio = hardware_map->cooling_relay_gpio;
    config->relay_polarity = cooling_relay_polarity_from_hardware(settings->relay_polarity);
    config->threshold_c_x10 = settings->threshold_c_x10;
    config->hysteresis_c_x10 = settings->hysteresis_c_x10;
    config->auto_enable = settings->auto_enable;
    config->mode = cooling_mode_from_hardware(settings->mode);
    config->test_timeout_sec = settings->test_timeout_sec;
    config->compressor_min_off_sec = settings->compressor_min_off_sec;
}

static void set_leds(int level)
{
    gpio_set_level(APP_CONFIG_LED_GPIO, level);
    gpio_set_level(APP_CONFIG_EXT_LED_GPIO, level);
}

static void hardware_ui_task(void *pvParameters)
{
    (void)pvParameters;
    
    // Initialize button GPIOs as inputs with internal pull-ups
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << APP_CONFIG_BOOT_BTN_GPIO) | (1ULL << APP_CONFIG_EXT_BTN_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_cfg);

    // Initialize LED GPIOs as outputs
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << APP_CONFIG_LED_GPIO) | (1ULL << APP_CONFIG_EXT_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_cfg);

    // Initial read to set up bootloader veto
    bool boot_btn_veto = (gpio_get_level(APP_CONFIG_BOOT_BTN_GPIO) == 0);
    bool ext_btn_veto = (gpio_get_level(APP_CONFIG_EXT_BTN_GPIO) == 0);

    if (boot_btn_veto) {
        ESP_LOGW(TAG, "Boot button active on boot; vetoing holds until released");
    }
    if (ext_btn_veto) {
        ESP_LOGW(TAG, "External button active on boot; vetoing holds until released");
    }

    uint32_t press_duration_ms = 0;
    bool recovery_ap_triggered = false;
    bool factory_reset_triggered = false;
    uint32_t release_ticks = 0;

    while (1) {
        int boot_level = gpio_get_level(APP_CONFIG_BOOT_BTN_GPIO);
        int ext_level = gpio_get_level(APP_CONFIG_EXT_BTN_GPIO);

        // Clear vetoes when buttons are released (level goes HIGH)
        if (boot_level == 1) {
            boot_btn_veto = false;
        }
        if (ext_level == 1) {
            ext_btn_veto = false;
        }

        bool boot_pressed = (boot_level == 0) && !boot_btn_veto;
        bool ext_pressed = (ext_level == 0) && !ext_btn_veto;

        // Mutual exclusion: if both are low, ignore both
        if (boot_pressed && ext_pressed) {
            boot_pressed = false;
            ext_pressed = false;
        }

        bool any_pressed = boot_pressed || ext_pressed;

        if (any_pressed) {
            press_duration_ms += 50;
            release_ticks = 0;

            if (press_duration_ms < 2000) {
                set_leds(1);
            } else if (press_duration_ms < 5000) {
                if (!recovery_ap_triggered) {
                    wifi_manager_start_recovery_ap();
                    recovery_ap_triggered = true;
                    ESP_LOGI(TAG, "Recovery AP triggered by button hold");
                }
                set_leds(((press_duration_ms / 500) % 2) == 0 ? 1 : 0);
            } else {
                if (!factory_reset_triggered) {
                    s_trigger_factory_reset = true;
                    factory_reset_triggered = true;
                    ESP_LOGI(TAG, "Factory reset triggered by button hold");
                }
                set_leds(((press_duration_ms / 100) % 2) == 0 ? 1 : 0);
            }
        } else {
            press_duration_ms = 0;
            recovery_ap_triggered = false;
            factory_reset_triggered = false;
            release_ticks++;

            uint8_t stg_type = 0;
            nvs_store_get_staging_type(&stg_type);
            if (stg_type > 0) {
                uint32_t phase = release_ticks % 20;
                if (phase < 2) {
                    set_leds(1);
                } else if (phase < 4) {
                    set_leds(0);
                } else if (phase < 6) {
                    set_leds(1);
                } else {
                    set_leds(0);
                }
            } else {
                if (wifi_manager_is_ap_active()) {
                    set_leds(1);
                } else {
                    set_leds(0);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  %s %s", APP_TEMPLATE_NAME, APP_TEMPLATE_FIRMWARE_VERSION);
    ESP_LOGI(TAG, "  %s", APP_TEMPLATE_PHASE_LABEL);
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Last reset reason: %s (%d)",
             reset_reason_name(esp_reset_reason()), esp_reset_reason());

    /* 1. Initialize NVS storage */
    if (!nvs_store_init()) {
        ESP_LOGE(TAG, "NVS init failed");
        return;
    }
    ESP_LOGI(TAG, "NVS initialized");

    uint8_t stg_type = 0;
    nvs_store_get_staging_type(&stg_type);
    if (stg_type > 0) {
        ESP_LOGW(TAG, "Staging configuration active (type=%d). Starting rollback timers.", stg_type);

        esp_timer_create_args_t confirm_args = {
            .callback = &confirm_timer_cb,
            .name = "confirm_timer"
        };
        if (esp_timer_create(&confirm_args, &s_confirm_timer) == ESP_OK) {
            esp_timer_start_once(s_confirm_timer, (uint64_t)APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS * 1000);
            ESP_LOGI(TAG, "Rollback confirmation timer started for %d ms", APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS);
        }

        if (stg_type == 1) {
            esp_timer_create_args_t wifi_args = {
                .callback = &wifi_timer_cb,
                .name = "wifi_timer"
            };
            if (esp_timer_create(&wifi_args, &s_wifi_timer) == ESP_OK) {
                esp_timer_start_once(s_wifi_timer, (uint64_t)APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS * 1000);
                ESP_LOGI(TAG, "Rollback Wi-Fi validation timer started for %d ms", APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS);
            }
        }
    }

    /* Initialize TFT display */
    if (tft_display_init() == ESP_OK) {
        tft_clear(TFT_COLOR_BLACK);
        // Draw centered high-contrast "Booting..." splash screen
        tft_draw_string_x2(80, 60, "Booting...", TFT_COLOR_GREEN, TFT_COLOR_BLACK);
        tft_draw_string(44, 110, APP_TEMPLATE_NAME, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
        tft_draw_string(136, 135, APP_TEMPLATE_FIRMWARE_VERSION, TFT_COLOR_GRAY, TFT_COLOR_BLACK);
    }

    hardware_map_t active_hardware_map;
    nvs_store_hardware_map_load_status_t hardware_map_status =
        nvs_store_load_hardware_map(&active_hardware_map);
    bool hardware_map_promoted = false;
    bool hardware_map_promotion_ok =
        promote_pending_hardware_map_on_boot(&active_hardware_map,
                                             &hardware_map_status,
                                             &hardware_map_promoted);
    bool hardware_reboot_required = false;
    bool hardware_reboot_status_ok = nvs_store_hardware_reboot_required(&hardware_reboot_required);

    nvs_store_pump_settings_t pump_settings;
    nvs_store_pump_settings_load_status_t pump_settings_status =
        nvs_store_load_pump_settings(&pump_settings);
    nvs_store_cooling_settings_t cooling_settings;
    nvs_store_cooling_settings_load_status_t cooling_settings_status =
        nvs_store_load_cooling_settings(&cooling_settings);

    pump_control_config_t pump_config = pump_control_default_config();
    apply_pump_settings_to_config(&pump_config, &pump_settings, &active_hardware_map);
    cooling_control_config_t cooling_config = cooling_control_default_config();
    apply_cooling_settings_to_config(&cooling_config, &cooling_settings, &active_hardware_map);

    bool allow_auto_start = pump_settings.auto_start;
    bool hardware_map_safe = hardware_map_status == NVS_STORE_HARDWARE_MAP_LOADED ||
                             hardware_map_status == NVS_STORE_HARDWARE_MAP_DEFAULTS_MISSING;
    if (pump_settings_status == NVS_STORE_PUMP_SETTINGS_LOADED) {
        ESP_LOGI(TAG, "Pump settings loaded from NVS");
    } else if (pump_settings_status == NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING) {
        ESP_LOGI(TAG, "Pump settings missing; using product defaults");
    } else {
        allow_auto_start = false;
        ESP_LOGW(TAG, "Pump settings %s; using defaults and suppressing auto-start for this boot",
                 pump_settings_load_status_name(pump_settings_status));
    }
    if (!hardware_map_safe) {
        allow_auto_start = false;
        ESP_LOGW(TAG, "Hardware map %s; using defaults and suppressing auto-start for this boot",
                 hardware_map_load_status_name(hardware_map_status));
    }
    if (!hardware_map_promotion_ok) {
        allow_auto_start = false;
        ESP_LOGW(TAG, "Pending hardware map promotion incomplete; suppressing auto-start for this boot");
    }

    ESP_LOGI(TAG,
             "Active hardware map (%s): float GPIO=%d, relay1 GPIO=%d, relay2 GPIO=%d, ds18b20 GPIO=%d, cooling GPIO=%d",
             hardware_map_load_status_name(hardware_map_status),
             active_hardware_map.float_input_gpio,
             active_hardware_map.pump_relay1_gpio,
             active_hardware_map.pump_relay2_gpio,
             active_hardware_map.ds18b20_data_gpio,
             active_hardware_map.cooling_relay_gpio);
    if (hardware_map_promoted) {
        ESP_LOGI(TAG, "Active hardware map was updated from pending values during this boot");
    }
    if (hardware_reboot_status_ok) {
        ESP_LOGI(TAG, "Pending hardware map reboot_required=%s",
                 hardware_reboot_required ? "true" : "false");
    } else {
        ESP_LOGW(TAG, "Pending hardware map status invalid; reboot-required state unavailable");
    }
    ESP_LOGI(TAG,
             "Cooling settings (%s): mode=%s, threshold_x10=%ld, hysteresis_x10=%ld, auto_enable=%s, min_off=%lu sec",
             cooling_settings_load_status_name(cooling_settings_status),
             hardware_map_cooling_mode_name(cooling_settings.mode),
             (long)cooling_settings.threshold_c_x10,
             (long)cooling_settings.hysteresis_c_x10,
             cooling_settings.auto_enable ? "true" : "false",
             (unsigned long)cooling_settings.compressor_min_off_sec);

    if (cooling_control_init(&cooling_config)) {
        ESP_LOGI(TAG,
                 "Cooling runtime initialized: ds18b20 GPIO=%d, relay GPIO=%d, auto_enable=%s, min_off=%lu sec",
                 cooling_config.ds18b20_gpio,
                 cooling_config.cooling_relay_gpio,
                 cooling_config.auto_enable ? "true" : "false",
                 (unsigned long)cooling_config.compressor_min_off_sec);
    } else {
        ESP_LOGE(TAG, "Cooling runtime init failed or safe-disabled; continuing with Wi-Fi/web setup");
    }

    if (pump_control_init(&pump_config)) {
        ESP_LOGI(TAG, "Pump control initialized: float GPIO=%d, relay1 GPIO=%d, relay2 GPIO=%d, auto_start=%s",
                 pump_config.float_gpio, pump_config.relay1_gpio, pump_config.relay2_gpio,
                 allow_auto_start ? "enabled" : "disabled");
        if (allow_auto_start) {
            if (pump_control_start()) {
                ESP_LOGI(TAG, "Pump control auto-started");
            } else {
                pump_control_stop();
                ESP_LOGE(TAG, "Pump control auto-start failed; continuing with Wi-Fi/web setup");
            }
        } else {
            ESP_LOGI(TAG, "Pump control initialized stopped with both pump relays inactive");
        }
    } else {
        ESP_LOGE(TAG, "Pump control init failed; continuing with Wi-Fi/web setup");
    }

    /* 2. Initialize session system */
    if (!session_init()) {
        ESP_LOGE(TAG, "Session system init failed; local web login unavailable");
    } else {
        ESP_LOGI(TAG, "Session system initialized");
    }

    /* 3. Initialize Wi-Fi (AP + STA with event handlers) */
    if (!wifi_manager_init()) {
        ESP_LOGE(TAG, "Wi-Fi init failed — continuing in AP-only mode");
    } else {
        ESP_LOGI(TAG, "Wi-Fi initialized");
    }
    ESP_LOGI(TAG, "AP SSID: %s, IP: %s", APP_TEMPLATE_AP_SSID, wifi_manager_get_ap_ip());

    /* 3a. Migrate legacy single-credential to multi-profile store (one-time, idempotent) */
    nvs_store_migrate_legacy_wifi();

    /* 3b. Auto-connect to the designated profile if one is configured */
    {
        wifi_profile_t profiles[WIFI_PROFILE_MAX] = {0};
        int prof_count = 0, auto_idx = -1;
        nvs_store_load_wifi_profiles(profiles, &prof_count, &auto_idx);
        if (auto_idx >= 0 && auto_idx < prof_count && profiles[auto_idx].ssid[0]) {
            ESP_LOGI(TAG, "Auto-connect to Wi-Fi profile[%d]: %s", auto_idx, profiles[auto_idx].ssid);
            wifi_manager_connect_sta(profiles[auto_idx].ssid, profiles[auto_idx].pass, NULL);
        } else if (prof_count == 0) {
            /* Fallback: try legacy sta_ssid only if no profile store entries exist yet */
            char sta_ssid[33] = {0}, sta_pass[65] = {0};
            if (nvs_store_load_wifi(sta_ssid, sizeof(sta_ssid), sta_pass, sizeof(sta_pass)) && sta_ssid[0]) {
                ESP_LOGI(TAG, "Auto-connect to legacy saved SSID: %s", sta_ssid);
                wifi_manager_connect_sta(sta_ssid, sta_pass, NULL);
            } else {
                ESP_LOGI(TAG, "No saved Wi-Fi credential; staying in AP-only mode");
            }
        } else {
            ESP_LOGI(TAG, "Wi-Fi auto-connect is disabled; staying in AP-only mode");
        }
    }

    /* 4. Start HTTP server */
    if (!web_server_start()) {
        ESP_LOGE(TAG, "HTTP server start failed — will retry");
        s_http_server_retry = true;
    } else {
        ESP_LOGI(TAG, "HTTP server started");
    }

    /* 5. Disable Wi-Fi power save (prevents mMC multicast loss) */
    esp_wifi_set_ps(WIFI_PS_NONE);

    /* 6. Initialize mDNS */
    esp_err_t mdns_err = mdns_init();
    if (mdns_err != ESP_OK) {
        ESP_LOGE(TAG, "mDNS init failed: %d", mdns_err);
    } else {
        mdns_hostname_set(APP_TEMPLATE_MDNS_HOSTNAME);
        mdns_instance_name_set(APP_TEMPLATE_MDNS_INSTANCE_NAME);
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
        ESP_LOGI(TAG, "mDNS initialized: http://%s.local", APP_TEMPLATE_MDNS_HOSTNAME);
    }

    /* 7. Start captive-portal DNS server (responds all queries with AP IP) */
    if (!dns_server_start()) {
        ESP_LOGE(TAG, "DNS server start failed");
    } else {
        ESP_LOGI(TAG, "DNS fallback server started (port 53)");
    }

    /* Start background TFT status update task */
    tft_display_start_task();

    // Start hardware UI/recovery task
    xTaskCreate(hardware_ui_task, "hardware_ui_task", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);

    /* 8. Initialize task watchdog (10s timeout with panic) */
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = APP_TEMPLATE_MAIN_WDT_TIMEOUT_MS,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_err_t wdt_ret = esp_task_wdt_init(&wdt_config);
    if (wdt_ret == ESP_OK) {
        esp_task_wdt_add(NULL);
        ESP_LOGI(TAG, "TWDT initialized (%d ms timeout, panic on trigger)",
                 APP_TEMPLATE_MAIN_WDT_TIMEOUT_MS);
    } else if (wdt_ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "TWDT already initialized, adding main task (timeout may differ)");
        esp_task_wdt_add(NULL);
    } else {
        ESP_LOGE(TAG, "TWDT init failed: %s (continuing without watchdog)",
                 esp_err_to_name(wdt_ret));
    }

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  System Ready");
    ESP_LOGI(TAG, "  AP SSID: %s", APP_TEMPLATE_AP_SSID);
    ESP_LOGI(TAG, "  AP IP: %s", wifi_manager_get_ap_ip());
    ESP_LOGI(TAG, "  URL: http://%s.local (mDNS)", APP_TEMPLATE_MDNS_HOSTNAME);
    ESP_LOGI(TAG, "  URL: http://%s (direct IP - always works)", wifi_manager_get_ap_ip());
    if (wifi_manager_is_sta_connected()) {
        ESP_LOGI(TAG, "  STA IP: %s", wifi_manager_get_sta_ip());
        ESP_LOGI(TAG, "  URL: http://%s.local (desktop/iOS on LAN)", APP_TEMPLATE_MDNS_HOSTNAME);
    }
    ESP_LOGI(TAG, "  Login user: %s", APP_TEMPLATE_DEFAULT_USERNAME);
    ESP_LOGI(TAG, "  WARNING: Change default credentials!");
    ESP_LOGI(TAG, "========================================");

    /* Main loop — reset WDT every 5s, retry HTTP server, log status every 30s */
    uint32_t loop_counter = 0;
    while (1) {
        esp_task_wdt_reset();

        if (s_cancel_rollback) {
            ESP_LOGI(TAG, "Staging confirmed. Stopping rollback timers.");
            if (s_confirm_timer) {
                esp_timer_stop(s_confirm_timer);
                esp_timer_delete(s_confirm_timer);
                s_confirm_timer = NULL;
            }
            if (s_wifi_timer) {
                esp_timer_stop(s_wifi_timer);
                esp_timer_delete(s_wifi_timer);
                s_wifi_timer = NULL;
            }
            s_cancel_rollback = false;
            s_trigger_rollback = false;
        }

        if (s_trigger_rollback) {
            uint8_t stg_type = 0;
            if (nvs_store_get_staging_type(&stg_type) && stg_type > 0) {
                ESP_LOGW(TAG, "Staged config timeout! Rolling back changes type=%d", stg_type);
                nvs_store_rollback_staging();
                session_invalidate_all();
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            }
            s_trigger_rollback = false;
        }

        if (s_trigger_factory_reset) {
            ESP_LOGW(TAG, "Factory reset requested by button hold. Resetting credentials...");
            if (nvs_store_factory_reset_credentials()) {
                ESP_LOGI(TAG, "Credentials reset to defaults successful");
            } else {
                ESP_LOGE(TAG, "Credentials reset to defaults failed");
            }
            session_invalidate_all();
            s_trigger_factory_reset = false;
        }

        if (s_http_server_retry) {
            if (web_server_start()) {
                ESP_LOGI(TAG, "HTTP server started on retry");
                s_http_server_retry = false;
            }
        }

        web_server_queue_health_check();
        if (!web_server_check_health(15000)) {
            ESP_LOGE(TAG, "FATAL: HTTP server task hang detected (>15s without processing work queue). Rebooting!");
            vTaskDelay(pdMS_TO_TICKS(100)); // Allow logs to flush
            esp_restart();
        }

        loop_counter++;
        if (loop_counter >= APP_TEMPLATE_STATUS_LOG_INTERVALS) {
            ESP_LOGI(TAG, "[STATUS] AP=%s, IP=%s, STA=%s, Heap=%lu, HeapMin=%lu, Largest=%lu, MainStackHWM=%lu",
                wifi_manager_is_ap_enabled() ? "ON" : "OFF",
                wifi_manager_get_ap_ip(),
                wifi_manager_is_sta_connected() ? wifi_manager_get_sta_ip() : "disconnected",
                (unsigned long)wifi_manager_get_free_heap(),
                (unsigned long)esp_get_minimum_free_heap_size(),
                (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                (unsigned long)uxTaskGetStackHighWaterMark(NULL));
            log_board_and_hardware_diagnostics();
            wifi_manager_log_diagnostics();
            web_server_log_diagnostics();
            loop_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

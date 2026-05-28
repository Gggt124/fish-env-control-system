#include "app_config.h"
#include "cooling_control.h"
#include "hardware_map.h"
#include "nvs_store.h"
#include "pump_control.h"
#include "wifi_manager.h"
#include "session.h"
#include "web_server.h"
#include "dns_server.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static const char *TAG = "app_main";
static bool s_http_server_retry = false;

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

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  %s %s", APP_TEMPLATE_NAME, APP_TEMPLATE_FIRMWARE_VERSION);
    ESP_LOGI(TAG, "  %s", APP_TEMPLATE_PHASE_LABEL);
    ESP_LOGI(TAG, "========================================");

    /* 1. Initialize NVS storage */
    if (!nvs_store_init()) {
        ESP_LOGE(TAG, "NVS init failed");
        return;
    }
    ESP_LOGI(TAG, "NVS initialized");

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
    session_init();
    ESP_LOGI(TAG, "Session system initialized");

    /* 3. Initialize Wi-Fi (AP + STA with event handlers) */
    if (!wifi_manager_init()) {
        ESP_LOGE(TAG, "Wi-Fi init failed — continuing in AP-only mode");
    } else {
        ESP_LOGI(TAG, "Wi-Fi initialized");
    }
    ESP_LOGI(TAG, "AP SSID: %s, IP: %s", APP_TEMPLATE_AP_SSID, wifi_manager_get_ap_ip());

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

    /* 8. Initialize task watchdog (30s timeout with panic) */
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

        if (s_http_server_retry) {
            if (web_server_start()) {
                ESP_LOGI(TAG, "HTTP server started on retry");
                s_http_server_retry = false;
            }
        }

        loop_counter++;
        if (loop_counter >= APP_TEMPLATE_STATUS_LOG_INTERVALS) {
            ESP_LOGI(TAG, "[STATUS] AP=%s, IP=%s, STA=%s, Heap=%lu",
                wifi_manager_is_ap_enabled() ? "ON" : "OFF",
                wifi_manager_get_ap_ip(),
                wifi_manager_is_sta_connected() ? wifi_manager_get_sta_ip() : "disconnected",
                (unsigned long)wifi_manager_get_free_heap());
            loop_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

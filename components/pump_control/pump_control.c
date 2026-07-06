#include "pump_control.h"

#include "app_config.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define PUMP_CONTROL_TICK_US 100000LL
#define MS_PER_SEC 1000LL

static const char *TAG = "pump_control";

static SemaphoreHandle_t s_pump_mutex;
static esp_timer_handle_t s_tick_timer;

static pump_control_config_t s_config;
static bool s_initialized;
static bool s_config_valid;
static bool s_running;
static bool s_initial_stabilizing;
static bool s_relay_configured;
static bool s_fault;
static bool s_relay_energized;
static bool s_relay1_energized;
static bool s_relay2_energized;

static pump_control_float_state_t s_confirmed_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
static pump_control_float_state_t s_pending_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
static int64_t s_pending_since_ms;

static pump_control_active_timer_t s_active_timer = PUMP_CONTROL_TIMER_NONE;
static pump_control_active_relay_t s_active_relay = PUMP_CONTROL_RELAY_NONE;
static pump_control_timer_phase_t s_phase = PUMP_CONTROL_PHASE_IDLE;
static uint32_t s_countdown_sec;
static int64_t s_phase_deadline_ms;



static bool polarity_valid(pump_control_relay_polarity_t polarity)
{
    return polarity == PUMP_CONTROL_RELAY_ACTIVE_HIGH ||
           polarity == PUMP_CONTROL_RELAY_ACTIVE_LOW;
}

static bool start_phase_valid(pump_control_start_phase_t phase)
{
    return phase == PUMP_CONTROL_START_PHASE_ON ||
           phase == PUMP_CONTROL_START_PHASE_OFF;
}

static int relay_active_level_for_polarity(pump_control_relay_polarity_t polarity)
{
    return polarity == PUMP_CONTROL_RELAY_ACTIVE_HIGH ? 1 : 0;
}

static int relay_inactive_level_for_polarity(pump_control_relay_polarity_t polarity)
{
    return relay_active_level_for_polarity(polarity) ? 0 : 1;
}

static bool gpio_valid_input(gpio_num_t gpio)
{
    return GPIO_IS_VALID_GPIO(gpio);
}

static bool gpio_valid_output(gpio_num_t gpio)
{
    return GPIO_IS_VALID_OUTPUT_GPIO(gpio);
}

static bool duration_valid(uint32_t seconds)
{
    return seconds >= APP_TEMPLATE_PUMP_TIMER_MIN_SEC &&
           seconds <= APP_TEMPLATE_PUMP_TIMER_MAX_SEC;
}

static bool timer_config_valid(const pump_control_timer_config_t *timer)
{
    return timer &&
           duration_valid(timer->on_sec) &&
           duration_valid(timer->off_sec);
}

static bool config_valid(const pump_control_config_t *config)
{
    if (!config) {
        return false;
    }
    if (!gpio_valid_input(config->float_gpio) ||
        !gpio_valid_output(config->relay1_gpio) ||
        !gpio_valid_output(config->relay2_gpio) ||
        config->float_gpio == config->relay1_gpio ||
        config->float_gpio == config->relay2_gpio ||
        config->relay1_gpio == config->relay2_gpio) {
        return false;
    }
    if (!polarity_valid(config->relay1_polarity) ||
        !polarity_valid(config->relay2_polarity) ||
        !start_phase_valid(config->timer1_start_phase) ||
        !start_phase_valid(config->timer2_start_phase) ||
        config->debounce_ms == 0) {
        return false;
    }
    return timer_config_valid(&config->timer1) &&
           timer_config_valid(&config->timer2);
}

static void reset_runtime_state_locked(void)
{
    s_running = false;
    s_initial_stabilizing = false;
    s_fault = false;
    s_relay_energized = false;
    s_relay1_energized = false;
    s_relay2_energized = false;
    s_confirmed_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
    s_pending_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
    s_pending_since_ms = 0;
    s_active_timer = PUMP_CONTROL_TIMER_NONE;
    s_active_relay = PUMP_CONTROL_RELAY_NONE;
    s_phase = PUMP_CONTROL_PHASE_IDLE;
    s_countdown_sec = 0;
    s_phase_deadline_ms = 0;
}

static gpio_num_t relay_gpio_for_config(const pump_control_config_t *config,
                                        pump_control_active_relay_t relay)
{
    if (!config) {
        return GPIO_NUM_NC;
    }
    if (relay == PUMP_CONTROL_RELAY_1) {
        return config->relay1_gpio;
    }
    if (relay == PUMP_CONTROL_RELAY_2) {
        return config->relay2_gpio;
    }
    return GPIO_NUM_NC;
}

static pump_control_relay_polarity_t relay_polarity_for_config(const pump_control_config_t *config,
                                                               pump_control_active_relay_t relay)
{
    if (!config) {
        return PUMP_CONTROL_RELAY_ACTIVE_LOW;
    }
    if (relay == PUMP_CONTROL_RELAY_1) {
        return config->relay1_polarity;
    }
    if (relay == PUMP_CONTROL_RELAY_2) {
        return config->relay2_polarity;
    }
    return PUMP_CONTROL_RELAY_ACTIVE_LOW;
}

static esp_err_t write_relay_level_for_config(const pump_control_config_t *config,
                                              pump_control_active_relay_t relay,
                                              bool energized)
{
    gpio_num_t gpio = relay_gpio_for_config(config, relay);
    if (!gpio_valid_output(gpio)) {
        return ESP_ERR_INVALID_ARG;
    }
    pump_control_relay_polarity_t polarity = relay_polarity_for_config(config, relay);
    int level = energized
        ? relay_active_level_for_polarity(polarity)
        : relay_inactive_level_for_polarity(polarity);
    return gpio_set_level(gpio, level);
}

static bool force_both_relays_inactive_for_config(const pump_control_config_t *config)
{
    bool ok = true;
    if (config && gpio_valid_output(config->relay1_gpio)) {
        ok = write_relay_level_for_config(config, PUMP_CONTROL_RELAY_1, false) == ESP_OK && ok;
    }
    if (config && gpio_valid_output(config->relay2_gpio)) {
        ok = write_relay_level_for_config(config, PUMP_CONTROL_RELAY_2, false) == ESP_OK && ok;
    }
    return ok;
}

static void mark_fault_locked(void)
{
    s_fault = true;
    s_config_valid = false;
    s_running = false;
    s_initial_stabilizing = false;
    s_relay_energized = false;
    s_relay1_energized = false;
    s_relay2_energized = false;
    s_phase = PUMP_CONTROL_PHASE_IDLE;
    s_countdown_sec = 0;
    s_phase_deadline_ms = 0;
    force_both_relays_inactive_for_config(&s_config);
}

static void force_both_relays_inactive_locked(void)
{
    s_relay_energized = false;
    s_relay1_energized = false;
    s_relay2_energized = false;
    if (!s_relay_configured) {
        return;
    }
    if (!force_both_relays_inactive_for_config(&s_config)) {
        mark_fault_locked();
    }
}

static void set_active_relay_energized_locked(bool energized)
{
    s_relay_energized = false;
    s_relay1_energized = false;
    s_relay2_energized = false;
    if (!s_relay_configured) {
        return;
    }
    if (!energized || s_active_relay == PUMP_CONTROL_RELAY_NONE) {
        force_both_relays_inactive_locked();
        return;
    }

    pump_control_active_relay_t inactive_relay =
        s_active_relay == PUMP_CONTROL_RELAY_1 ? PUMP_CONTROL_RELAY_2 : PUMP_CONTROL_RELAY_1;
    if (write_relay_level_for_config(&s_config, inactive_relay, false) != ESP_OK ||
        write_relay_level_for_config(&s_config, s_active_relay, true) != ESP_OK) {
        mark_fault_locked();
        return;
    }

    s_relay_energized = true;
    if (s_active_relay == PUMP_CONTROL_RELAY_1) {
        s_relay1_energized = true;
    } else if (s_active_relay == PUMP_CONTROL_RELAY_2) {
        s_relay2_energized = true;
    }
}

static bool configure_gpio_locked(const pump_control_config_t *config)
{
    if (!force_both_relays_inactive_for_config(config)) {
        return false;
    }

    gpio_config_t relay_cfg = {
        .pin_bit_mask = (1ULL << (uint32_t)config->relay1_gpio) |
                        (1ULL << (uint32_t)config->relay2_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&relay_cfg) != ESP_OK) {
        return false;
    }
    if (!force_both_relays_inactive_for_config(config)) {
        return false;
    }

    if (config->float_gpio >= 34 && config->float_gpio <= 39) {
        ESP_LOGW(TAG, "Float GPIO %d is input-only. Internal pull-up/down will not work. Ensure external resistor is installed!", config->float_gpio);
    }

    gpio_config_t float_cfg = {
        .pin_bit_mask = (1ULL << (uint32_t)config->float_gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = config->float_active_low ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = config->float_active_low ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&float_cfg) == ESP_OK;
}

static pump_control_float_state_t read_float_state_locked(void)
{
    int level = gpio_get_level(s_config.float_gpio);
    bool active = s_config.float_active_low ? (level == 0) : (level != 0);
    return active ? PUMP_CONTROL_FLOAT_ON : PUMP_CONTROL_FLOAT_OFF;
}

static uint32_t seconds_remaining(int64_t now_ms)
{
    if (!s_running || s_phase == PUMP_CONTROL_PHASE_IDLE || s_phase_deadline_ms <= now_ms) {
        return 0;
    }

    int64_t remaining_ms = s_phase_deadline_ms - now_ms;
    return (uint32_t)((remaining_ms + MS_PER_SEC - 1) / MS_PER_SEC);
}

static pump_control_active_relay_t active_relay_for_timer(pump_control_active_timer_t timer)
{
    if (timer == PUMP_CONTROL_TIMER_1) {
        return PUMP_CONTROL_RELAY_1;
    }
    if (timer == PUMP_CONTROL_TIMER_2) {
        return PUMP_CONTROL_RELAY_2;
    }
    return PUMP_CONTROL_RELAY_NONE;
}

static gpio_num_t active_relay_gpio_locked(void)
{
    if (s_active_relay == PUMP_CONTROL_RELAY_2) {
        return s_config.relay2_gpio;
    }
    return s_config.relay1_gpio;
}

static const pump_control_timer_config_t *active_timer_config_locked(void)
{
    switch (s_active_timer) {
    case PUMP_CONTROL_TIMER_1:
        return &s_config.timer1;
    case PUMP_CONTROL_TIMER_2:
        return &s_config.timer2;
    default:
        return NULL;
    }
}

static pump_control_start_phase_t active_timer_start_phase_locked(void)
{
    if (s_active_timer == PUMP_CONTROL_TIMER_2) {
        return s_config.timer2_start_phase;
    }
    return s_config.timer1_start_phase;
}

static pump_control_timer_phase_t start_phase_to_timer_phase(pump_control_start_phase_t phase)
{
    return phase == PUMP_CONTROL_START_PHASE_OFF
        ? PUMP_CONTROL_PHASE_OFF
        : PUMP_CONTROL_PHASE_ON;
}

static void set_phase_locked(pump_control_timer_phase_t phase, int64_t now_ms)
{
    const pump_control_timer_config_t *timer = active_timer_config_locked();
    if (!timer) {
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
        force_both_relays_inactive_locked();
        return;
    }

    uint32_t duration_sec = (phase == PUMP_CONTROL_PHASE_ON) ? timer->on_sec : timer->off_sec;
    s_phase = phase;
    s_phase_deadline_ms = now_ms + ((int64_t)duration_sec * MS_PER_SEC);
    s_countdown_sec = duration_sec;
    set_active_relay_energized_locked(phase == PUMP_CONTROL_PHASE_ON);
}

static void set_selected_channel_for_float_locked(pump_control_float_state_t float_state)
{
    // v1.1 mapping: Float OFF selects Timer 1 / Relay 1; Float ON selects Timer 2 / Relay 2.
    if (float_state == PUMP_CONTROL_FLOAT_ON) {
        s_active_timer = PUMP_CONTROL_TIMER_2;
    } else if (float_state == PUMP_CONTROL_FLOAT_OFF) {
        s_active_timer = PUMP_CONTROL_TIMER_1;
    } else {
        s_active_timer = PUMP_CONTROL_TIMER_NONE;
    }
    s_active_relay = active_relay_for_timer(s_active_timer);
}

static void preview_channel_for_float_locked(pump_control_float_state_t float_state)
{
    set_selected_channel_for_float_locked(float_state);
    if (s_active_timer == PUMP_CONTROL_TIMER_NONE) {
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
        s_phase_deadline_ms = 0;
        force_both_relays_inactive_locked();
        return;
    }

    const pump_control_timer_config_t *timer = active_timer_config_locked();
    pump_control_timer_phase_t phase =
        start_phase_to_timer_phase(active_timer_start_phase_locked());
    s_phase = phase;
    s_phase_deadline_ms = 0;
    s_countdown_sec = phase == PUMP_CONTROL_PHASE_ON ? timer->on_sec : timer->off_sec;
    force_both_relays_inactive_locked();
}

static void start_channel_for_float_locked(pump_control_float_state_t float_state, int64_t now_ms)
{
    force_both_relays_inactive_locked();
    if (s_fault) {
        return;
    }

    set_selected_channel_for_float_locked(float_state);
    if (s_active_timer == PUMP_CONTROL_TIMER_NONE) {
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
        s_phase_deadline_ms = 0;
        force_both_relays_inactive_locked();
        return;
    }

    set_phase_locked(start_phase_to_timer_phase(active_timer_start_phase_locked()), now_ms);
}

static bool update_debounce_locked(pump_control_float_state_t raw_state, int64_t now_ms)
{
    if (s_pending_float_state != raw_state) {
        s_pending_float_state = raw_state;
        s_pending_since_ms = now_ms;
        return false;
    }

    if (s_confirmed_float_state == raw_state) {
        return false;
    }

    if ((uint32_t)(now_ms - s_pending_since_ms) < s_config.debounce_ms) {
        return false;
    }

    s_confirmed_float_state = raw_state;
    return true;
}

static void advance_phase_if_needed_locked(int64_t now_ms)
{
    if (!s_running || s_phase == PUMP_CONTROL_PHASE_IDLE || s_phase_deadline_ms > now_ms) {
        s_countdown_sec = seconds_remaining(now_ms);
        return;
    }

    if (s_phase == PUMP_CONTROL_PHASE_ON) {
        set_phase_locked(PUMP_CONTROL_PHASE_OFF, now_ms);
    } else {
        set_phase_locked(PUMP_CONTROL_PHASE_ON, now_ms);
    }
}

static void pump_tick_cb(void *arg)
{
    (void)arg;

    if (!s_pump_mutex || xSemaphoreTake(s_pump_mutex, 0) != pdTRUE) {
        return;
    }

    if (!s_initialized || !s_config_valid || s_fault) {
        xSemaphoreGive(s_pump_mutex);
        return;
    }

    int64_t now_ms = esp_timer_get_time() / 1000;
    pump_control_float_state_t raw_state = read_float_state_locked();
    bool float_changed = update_debounce_locked(raw_state, now_ms);

    if (!s_running) {
        preview_channel_for_float_locked(s_confirmed_float_state);
        xSemaphoreGive(s_pump_mutex);
        return;
    }

    if (s_initial_stabilizing) {
        force_both_relays_inactive_locked();
        if (s_confirmed_float_state != PUMP_CONTROL_FLOAT_UNKNOWN) {
            s_initial_stabilizing = false;
            start_channel_for_float_locked(s_confirmed_float_state, now_ms);
        }
        xSemaphoreGive(s_pump_mutex);
        return;
    }

    if (float_changed) {
        start_channel_for_float_locked(s_confirmed_float_state, now_ms);
    } else {
        advance_phase_if_needed_locked(now_ms);
    }

    s_countdown_sec = seconds_remaining(now_ms);
    xSemaphoreGive(s_pump_mutex);
}

static bool ensure_timer_locked(void)
{
    if (s_tick_timer) {
        return true;
    }

    const esp_timer_create_args_t timer_args = {
        .callback = pump_tick_cb,
        .arg = NULL,
        .name = "pump_control_tick",
    };
    return esp_timer_create(&timer_args, &s_tick_timer) == ESP_OK;
}

pump_control_config_t pump_control_default_config(void)
{
    pump_control_relay_polarity_t relay1_polarity = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_ACTIVE_LOW
        ? PUMP_CONTROL_RELAY_ACTIVE_LOW
        : PUMP_CONTROL_RELAY_ACTIVE_HIGH;
    pump_control_config_t config = {
        .float_gpio = APP_TEMPLATE_PUMP_FLOAT_GPIO,
        .float_active_low = APP_TEMPLATE_PUMP_FLOAT_ACTIVE_LOW,
        .relay_gpio = APP_TEMPLATE_PUMP_RELAY_GPIO,
        .relay_polarity = relay1_polarity,
        .relay1_gpio = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_GPIO,
        .relay2_gpio = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_GPIO,
        .relay1_polarity = relay1_polarity,
        .relay2_polarity = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_ACTIVE_LOW
            ? PUMP_CONTROL_RELAY_ACTIVE_LOW
            : PUMP_CONTROL_RELAY_ACTIVE_HIGH,
        .debounce_ms = APP_TEMPLATE_PUMP_FLOAT_DEBOUNCE_MS,
        .timer1 = {
            .on_sec = APP_TEMPLATE_PUMP_TIMER1_ON_SEC,
            .off_sec = APP_TEMPLATE_PUMP_TIMER1_OFF_SEC,
        },
        .timer2 = {
            .on_sec = APP_TEMPLATE_PUMP_TIMER2_ON_SEC,
            .off_sec = APP_TEMPLATE_PUMP_TIMER2_OFF_SEC,
        },
        .timer1_start_phase = APP_TEMPLATE_PUMP_TIMER_START_ON
            ? PUMP_CONTROL_START_PHASE_ON
            : PUMP_CONTROL_START_PHASE_OFF,
        .timer2_start_phase = APP_TEMPLATE_PUMP_TIMER_START_ON
            ? PUMP_CONTROL_START_PHASE_ON
            : PUMP_CONTROL_START_PHASE_OFF,
    };
    return config;
}

bool pump_control_init(const pump_control_config_t *config)
{
    if (!config) {
        return false;
    }

    if (!s_pump_mutex) {
        s_pump_mutex = xSemaphoreCreateMutex();
        if (!s_pump_mutex) {
            ESP_LOGE(TAG, "Failed to create pump mutex");
            return false;
        }
    }

    if (xSemaphoreTake(s_pump_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }

    if (s_tick_timer) {
        esp_timer_stop(s_tick_timer);
    }

    s_config = *config;
    s_config.relay_gpio = s_config.relay1_gpio;
    s_config.relay_polarity = s_config.relay1_polarity;
    s_initialized = false;
    s_config_valid = false;
    s_relay_configured = false;
    reset_runtime_state_locked();

    if (!config_valid(&s_config)) {
        force_both_relays_inactive_for_config(&s_config);
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    if (!configure_gpio_locked(&s_config)) {
        force_both_relays_inactive_for_config(&s_config);
        reset_runtime_state_locked();
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    s_relay_configured = true;
    force_both_relays_inactive_locked();

    if (!ensure_timer_locked()) {
        force_both_relays_inactive_for_config(&s_config);
        s_relay_configured = false;
        reset_runtime_state_locked();
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    esp_err_t start_err = esp_timer_start_periodic(s_tick_timer, PUMP_CONTROL_TICK_US);
    if (start_err != ESP_OK) {
        force_both_relays_inactive_for_config(&s_config);
        s_relay_configured = false;
        reset_runtime_state_locked();
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    s_initialized = true;
    s_config_valid = true;
    xSemaphoreGive(s_pump_mutex);
    return true;
}

bool pump_control_start(void)
{
    if (!s_pump_mutex) {
        ESP_LOGE(TAG, "pump_control not initialized");
        return false;
    }

    if (xSemaphoreTake(s_pump_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }
    if (!s_initialized || !s_config_valid || s_fault) {
        xSemaphoreGive(s_pump_mutex);
        return false;
    }
    if (s_running) {
        xSemaphoreGive(s_pump_mutex);
        return true;
    }

    s_running = true;
    s_initial_stabilizing = s_confirmed_float_state == PUMP_CONTROL_FLOAT_UNKNOWN;
    s_phase_deadline_ms = 0;
    force_both_relays_inactive_locked();
    if (!s_initial_stabilizing) {
        int64_t now_ms = esp_timer_get_time() / 1000;
        start_channel_for_float_locked(s_confirmed_float_state, now_ms);
    } else {
        s_active_timer = PUMP_CONTROL_TIMER_NONE;
        s_active_relay = PUMP_CONTROL_RELAY_NONE;
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
    }

    xSemaphoreGive(s_pump_mutex);
    return true;
}

bool pump_control_stop(void)
{
    if (!s_pump_mutex) {
        ESP_LOGE(TAG, "pump_control not initialized");
        return false;
    }

    if (xSemaphoreTake(s_pump_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }
    if (!s_initialized || !s_config_valid) {
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    s_running = false;
    s_initial_stabilizing = false;
    s_phase_deadline_ms = 0;
    force_both_relays_inactive_locked();
    preview_channel_for_float_locked(s_confirmed_float_state);
    xSemaphoreGive(s_pump_mutex);
    return true;
}

bool pump_control_update_timers(const pump_control_timer_update_t *update)
{
    if (!update) {
        return false;
    }
    if (!s_pump_mutex) {
        ESP_LOGE(TAG, "pump_control not initialized");
        return false;
    }

    /* Validate all fields before taking the lock */
    if (!timer_config_valid(&update->timer1) ||
        !timer_config_valid(&update->timer2) ||
        !polarity_valid(update->relay1_polarity) ||
        !polarity_valid(update->relay2_polarity) ||
        !start_phase_valid(update->timer1_start_phase) ||
        !start_phase_valid(update->timer2_start_phase)) {
        ESP_LOGE(TAG, "pump_control_update_timers: invalid update fields");
        return false;
    }

    if (xSemaphoreTake(s_pump_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }

    if (!s_initialized || !s_config_valid || s_fault) {
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    /* Apply soft config in-place — GPIO, relay state, and phase are untouched */
    s_config.timer1             = update->timer1;
    s_config.timer2             = update->timer2;
    s_config.timer1_start_phase = update->timer1_start_phase;
    s_config.timer2_start_phase = update->timer2_start_phase;
    s_config.relay1_polarity    = update->relay1_polarity;
    s_config.relay2_polarity    = update->relay2_polarity;
    s_config.relay_polarity     = update->relay1_polarity; /* legacy alias */

    ESP_LOGI(TAG, "Timer config updated live: t1=%lus/%lus t2=%lus/%lus",
             (unsigned long)update->timer1.on_sec,
             (unsigned long)update->timer1.off_sec,
             (unsigned long)update->timer2.on_sec,
             (unsigned long)update->timer2.off_sec);

    xSemaphoreGive(s_pump_mutex);
    return true;
}

bool pump_control_get_status(pump_control_status_t *out)
{
    if (!s_pump_mutex) {
        ESP_LOGW(TAG, "pump_control not initialized");
        return false;
    }
    if (!out) {
        return false;
    }

    if (xSemaphoreTake(s_pump_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }
    if (s_running) {
        int64_t now_ms = esp_timer_get_time() / 1000;
        s_countdown_sec = seconds_remaining(now_ms);
    } else if (s_initialized && s_config_valid && !s_fault) {
        preview_channel_for_float_locked(s_confirmed_float_state);
    }

    out->initialized = s_initialized;
    out->config_valid = s_config_valid;
    out->running = s_running;
    out->initial_stabilizing = s_initial_stabilizing;
    out->fault = s_fault;
    out->relay_energized = s_relay_energized;
    out->relay1_energized = s_relay1_energized;
    out->relay2_energized = s_relay2_energized;
    out->float_gpio = s_config.float_gpio;
    out->relay_gpio = active_relay_gpio_locked();
    out->relay1_gpio = s_config.relay1_gpio;
    out->relay2_gpio = s_config.relay2_gpio;
    out->float_state = s_confirmed_float_state;
    out->active_timer = s_active_timer;
    out->active_relay = s_active_relay;
    out->phase = s_phase;
    out->countdown_sec = s_countdown_sec;
    out->total_duration_sec = 0;
    const pump_control_timer_config_t *timer = active_timer_config_locked();
    if (timer && (s_phase == PUMP_CONTROL_PHASE_ON || s_phase == PUMP_CONTROL_PHASE_OFF)) {
        out->total_duration_sec = s_phase == PUMP_CONTROL_PHASE_ON ? timer->on_sec : timer->off_sec;
    }

    xSemaphoreGive(s_pump_mutex);
    return true;
}

#include "pump_control.h"

#include <string.h>
#include "app_config.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define PUMP_CONTROL_TICK_US 100000LL
#define MS_PER_SEC 1000LL

static SemaphoreHandle_t s_pump_mutex;
static esp_timer_handle_t s_tick_timer;

static pump_control_config_t s_config;
static bool s_initialized;
static bool s_config_valid;
static bool s_running;
static bool s_initial_stabilizing;
static bool s_relay_configured;
static bool s_relay_energized;

static pump_control_float_state_t s_confirmed_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
static pump_control_float_state_t s_pending_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
static int64_t s_pending_since_ms;

static pump_control_active_timer_t s_active_timer = PUMP_CONTROL_TIMER_NONE;
static pump_control_timer_phase_t s_phase = PUMP_CONTROL_PHASE_IDLE;
static uint32_t s_countdown_sec;
static int64_t s_phase_deadline_ms;

static bool ensure_mutex(void)
{
    if (s_pump_mutex) {
        return true;
    }

    s_pump_mutex = xSemaphoreCreateMutex();
    return s_pump_mutex != NULL;
}

static bool polarity_valid(pump_control_relay_polarity_t polarity)
{
    return polarity == PUMP_CONTROL_RELAY_ACTIVE_HIGH ||
           polarity == PUMP_CONTROL_RELAY_ACTIVE_LOW;
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
        !gpio_valid_output(config->relay_gpio) ||
        config->float_gpio == config->relay_gpio) {
        return false;
    }
    if (!polarity_valid(config->relay_polarity) || config->debounce_ms == 0) {
        return false;
    }
    return timer_config_valid(&config->timer1) &&
           timer_config_valid(&config->timer2);
}

static void reset_runtime_state_locked(void)
{
    s_running = false;
    s_initial_stabilizing = false;
    s_relay_energized = false;
    s_confirmed_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
    s_pending_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
    s_pending_since_ms = 0;
    s_active_timer = PUMP_CONTROL_TIMER_NONE;
    s_phase = PUMP_CONTROL_PHASE_IDLE;
    s_countdown_sec = 0;
    s_phase_deadline_ms = 0;
}

static esp_err_t write_relay_inactive_for_config(const pump_control_config_t *config)
{
    if (!config || !gpio_valid_output(config->relay_gpio)) {
        return ESP_ERR_INVALID_ARG;
    }
    int inactive_level = relay_inactive_level_for_polarity(config->relay_polarity);
    return gpio_set_level(config->relay_gpio, inactive_level);
}

static void set_relay_energized_locked(bool energized)
{
    s_relay_energized = false;
    if (!s_relay_configured) {
        return;
    }

    int level = energized
        ? relay_active_level_for_polarity(s_config.relay_polarity)
        : relay_inactive_level_for_polarity(s_config.relay_polarity);

    if (gpio_set_level(s_config.relay_gpio, level) == ESP_OK) {
        s_relay_energized = energized;
    }
}

static bool configure_gpio_locked(const pump_control_config_t *config)
{
    if (write_relay_inactive_for_config(config) != ESP_OK) {
        return false;
    }

    gpio_config_t relay_cfg = {
        .pin_bit_mask = (1ULL << (uint32_t)config->relay_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&relay_cfg) != ESP_OK) {
        return false;
    }
    if (write_relay_inactive_for_config(config) != ESP_OK) {
        return false;
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

static void set_phase_locked(pump_control_timer_phase_t phase, int64_t now_ms)
{
    const pump_control_timer_config_t *timer = active_timer_config_locked();
    if (!timer) {
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
        set_relay_energized_locked(false);
        return;
    }

    uint32_t duration_sec = (phase == PUMP_CONTROL_PHASE_ON) ? timer->on_sec : timer->off_sec;
    s_phase = phase;
    s_phase_deadline_ms = now_ms + ((int64_t)duration_sec * MS_PER_SEC);
    s_countdown_sec = duration_sec;
    set_relay_energized_locked(phase == PUMP_CONTROL_PHASE_ON);
}

static void select_timer_for_float_locked(pump_control_float_state_t float_state, int64_t now_ms)
{
    if (float_state == PUMP_CONTROL_FLOAT_ON) {
        s_active_timer = PUMP_CONTROL_TIMER_2;
    } else if (float_state == PUMP_CONTROL_FLOAT_OFF) {
        s_active_timer = PUMP_CONTROL_TIMER_1;
    } else {
        s_active_timer = PUMP_CONTROL_TIMER_NONE;
    }

    if (s_active_timer == PUMP_CONTROL_TIMER_NONE) {
        s_phase = PUMP_CONTROL_PHASE_IDLE;
        s_countdown_sec = 0;
        set_relay_energized_locked(false);
        return;
    }

    set_phase_locked(PUMP_CONTROL_PHASE_ON, now_ms);
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

    if (!s_initialized || !s_config_valid || !s_running) {
        xSemaphoreGive(s_pump_mutex);
        return;
    }

    int64_t now_ms = esp_timer_get_time() / 1000;
    pump_control_float_state_t raw_state = read_float_state_locked();
    bool float_changed = update_debounce_locked(raw_state, now_ms);

    if (s_initial_stabilizing) {
        set_relay_energized_locked(false);
        if (s_confirmed_float_state != PUMP_CONTROL_FLOAT_UNKNOWN) {
            s_initial_stabilizing = false;
            select_timer_for_float_locked(s_confirmed_float_state, now_ms);
        }
        xSemaphoreGive(s_pump_mutex);
        return;
    }

    if (float_changed) {
        select_timer_for_float_locked(s_confirmed_float_state, now_ms);
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
    pump_control_config_t config = {
        .float_gpio = APP_TEMPLATE_PUMP_FLOAT_GPIO,
        .float_active_low = APP_TEMPLATE_PUMP_FLOAT_ACTIVE_LOW,
        .relay_gpio = APP_TEMPLATE_PUMP_RELAY_GPIO,
        .relay_polarity = APP_TEMPLATE_PUMP_RELAY_ACTIVE_LOW
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
    };
    return config;
}

bool pump_control_init(const pump_control_config_t *config)
{
    if (!config || !ensure_mutex()) {
        return false;
    }

    xSemaphoreTake(s_pump_mutex, portMAX_DELAY);

    if (s_tick_timer) {
        esp_timer_stop(s_tick_timer);
    }

    s_config = *config;
    s_initialized = false;
    s_config_valid = false;
    s_relay_configured = false;
    reset_runtime_state_locked();

    if (!config_valid(config)) {
        write_relay_inactive_for_config(config);
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    if (!configure_gpio_locked(config)) {
        write_relay_inactive_for_config(config);
        reset_runtime_state_locked();
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    s_relay_configured = true;
    set_relay_energized_locked(false);

    if (!ensure_timer_locked()) {
        write_relay_inactive_for_config(config);
        s_relay_configured = false;
        reset_runtime_state_locked();
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    esp_err_t start_err = esp_timer_start_periodic(s_tick_timer, PUMP_CONTROL_TICK_US);
    if (start_err != ESP_OK) {
        write_relay_inactive_for_config(config);
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
    if (!ensure_mutex()) {
        return false;
    }

    xSemaphoreTake(s_pump_mutex, portMAX_DELAY);
    if (!s_initialized || !s_config_valid) {
        xSemaphoreGive(s_pump_mutex);
        return false;
    }
    if (s_running) {
        xSemaphoreGive(s_pump_mutex);
        return true;
    }

    s_running = true;
    s_initial_stabilizing = true;
    s_confirmed_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
    s_pending_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
    s_pending_since_ms = 0;
    s_active_timer = PUMP_CONTROL_TIMER_NONE;
    s_phase = PUMP_CONTROL_PHASE_IDLE;
    s_countdown_sec = 0;
    s_phase_deadline_ms = 0;
    set_relay_energized_locked(false);

    xSemaphoreGive(s_pump_mutex);
    return true;
}

bool pump_control_stop(void)
{
    if (!ensure_mutex()) {
        return false;
    }

    xSemaphoreTake(s_pump_mutex, portMAX_DELAY);
    if (!s_initialized || !s_config_valid) {
        xSemaphoreGive(s_pump_mutex);
        return false;
    }

    reset_runtime_state_locked();
    set_relay_energized_locked(false);
    xSemaphoreGive(s_pump_mutex);
    return true;
}

bool pump_control_get_status(pump_control_status_t *out)
{
    if (!out || !ensure_mutex()) {
        return false;
    }

    xSemaphoreTake(s_pump_mutex, portMAX_DELAY);
    int64_t now_ms = esp_timer_get_time() / 1000;
    s_countdown_sec = seconds_remaining(now_ms);

    out->initialized = s_initialized;
    out->config_valid = s_config_valid;
    out->running = s_running;
    out->initial_stabilizing = s_initial_stabilizing;
    out->relay_energized = s_relay_energized;
    out->float_gpio = s_config.float_gpio;
    out->relay_gpio = s_config.relay_gpio;
    out->float_state = s_confirmed_float_state;
    out->active_timer = s_active_timer;
    out->phase = s_phase;
    out->countdown_sec = s_countdown_sec;

    xSemaphoreGive(s_pump_mutex);
    return true;
}

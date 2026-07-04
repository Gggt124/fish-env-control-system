#include "hardware_map.h"

#include "app_config.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* === RESERVED GPIO PINS — must NOT appear in any option array ===
 *
 * TFT SPI display (APP_TEMPLATE_TFT_PIN_*):
 *   GPIO  5 (CS)   18 (CLK)   21 (DC)   22 (RST)   23 (MOSI)
 *
 * Factory Reset / Status UI (APP_CONFIG_*_GPIO in app_config.h):
 *   GPIO  0  — Boot/Factory-Reset button  (strapping pin; BOOT_BTN)
 *   GPIO  2  — Status LED                 (strapping pin; LED)
 *   GPIO 13  — External status LED        (EXT_LED)
 *   GPIO 14  — External Factory-Reset btn (EXT_BTN)
 *
 * None of the above may be offered to the user as selectable GPIO.
 * If APP_CONFIG_* values change in app_config.h, update this list too.
 * =================================================================== */

static const hardware_gpio_option_t s_float_options[] = {
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_32, true, true, true, true, "GPIO32"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_33, true, true, true, false, "GPIO33"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_25, true, true, true, false, "GPIO25"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_26, true, true, true, false, "GPIO26"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_27, true, true, true, false, "GPIO27"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_34, true, false, false, false, "GPIO34 external pull required"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_35, true, false, false, false, "GPIO35 external pull required"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_36, true, false, false, false, "GPIO36 external pull required"},
    {HARDWARE_ROLE_FLOAT_INPUT, GPIO_NUM_39, true, false, false, false, "GPIO39 external pull required"},
};

static const hardware_gpio_option_t s_pump_relay1_options[] = {
    {HARDWARE_ROLE_PUMP_RELAY_1, GPIO_NUM_26, true, true, true, true, "GPIO26"},
    {HARDWARE_ROLE_PUMP_RELAY_1, GPIO_NUM_27, true, true, true, false, "GPIO27"},
    {HARDWARE_ROLE_PUMP_RELAY_1, GPIO_NUM_25, true, true, true, false, "GPIO25"},
    {HARDWARE_ROLE_PUMP_RELAY_1, GPIO_NUM_32, true, true, true, false, "GPIO32"},
    {HARDWARE_ROLE_PUMP_RELAY_1, GPIO_NUM_33, true, true, true, false, "GPIO33"},
    {HARDWARE_ROLE_PUMP_RELAY_1, GPIO_NUM_19, true, true, true, false, "GPIO19"},
};

static const hardware_gpio_option_t s_pump_relay2_options[] = {
    {HARDWARE_ROLE_PUMP_RELAY_2, GPIO_NUM_27, true, true, true, true, "GPIO27"},
    {HARDWARE_ROLE_PUMP_RELAY_2, GPIO_NUM_26, true, true, true, false, "GPIO26"},
    {HARDWARE_ROLE_PUMP_RELAY_2, GPIO_NUM_25, true, true, true, false, "GPIO25"},
    {HARDWARE_ROLE_PUMP_RELAY_2, GPIO_NUM_32, true, true, true, false, "GPIO32"},
    {HARDWARE_ROLE_PUMP_RELAY_2, GPIO_NUM_33, true, true, true, false, "GPIO33"},
    {HARDWARE_ROLE_PUMP_RELAY_2, GPIO_NUM_19, true, true, true, false, "GPIO19"},
};

static const hardware_gpio_option_t s_ds18b20_options[] = {
    {HARDWARE_ROLE_DS18B20_DATA, GPIO_NUM_33, true, true, true, true, "GPIO33"},
    {HARDWARE_ROLE_DS18B20_DATA, GPIO_NUM_32, true, true, true, false, "GPIO32"},
    {HARDWARE_ROLE_DS18B20_DATA, GPIO_NUM_25, true, true, true, false, "GPIO25"},
    {HARDWARE_ROLE_DS18B20_DATA, GPIO_NUM_26, true, true, true, false, "GPIO26"},
    {HARDWARE_ROLE_DS18B20_DATA, GPIO_NUM_27, true, true, true, false, "GPIO27"},
    {HARDWARE_ROLE_DS18B20_DATA, GPIO_NUM_19, true, true, true, false, "GPIO19"},
};

static const hardware_gpio_option_t s_cooling_relay_options[] = {
    {HARDWARE_ROLE_COOLING_RELAY, GPIO_NUM_25, true, true, true, true, "GPIO25"},
    {HARDWARE_ROLE_COOLING_RELAY, GPIO_NUM_26, true, true, true, false, "GPIO26"},
    {HARDWARE_ROLE_COOLING_RELAY, GPIO_NUM_27, true, true, true, false, "GPIO27"},
    {HARDWARE_ROLE_COOLING_RELAY, GPIO_NUM_32, true, true, true, false, "GPIO32"},
    {HARDWARE_ROLE_COOLING_RELAY, GPIO_NUM_33, true, true, true, false, "GPIO33"},
    {HARDWARE_ROLE_COOLING_RELAY, GPIO_NUM_19, true, true, true, false, "GPIO19"},
};

hardware_map_t hardware_map_defaults(void)
{
    return (hardware_map_t) {
        .float_input_gpio = APP_TEMPLATE_HW_DEFAULT_FLOAT_GPIO,
        .pump_relay1_gpio = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_GPIO,
        .pump_relay2_gpio = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_GPIO,
        .ds18b20_data_gpio = APP_TEMPLATE_HW_DEFAULT_DS18B20_GPIO,
        .cooling_relay_gpio = APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_GPIO,
    };
}

hardware_relay_polarity_map_t hardware_map_default_polarities(void)
{
    return (hardware_relay_polarity_map_t) {
        .pump_relay1 = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY1_ACTIVE_LOW
            ? HARDWARE_RELAY_ACTIVE_LOW
            : HARDWARE_RELAY_ACTIVE_HIGH,
        .pump_relay2 = APP_TEMPLATE_HW_DEFAULT_PUMP_RELAY2_ACTIVE_LOW
            ? HARDWARE_RELAY_ACTIVE_LOW
            : HARDWARE_RELAY_ACTIVE_HIGH,
        .cooling_relay = APP_TEMPLATE_HW_DEFAULT_COOLING_RELAY_ACTIVE_LOW
            ? HARDWARE_RELAY_ACTIVE_LOW
            : HARDWARE_RELAY_ACTIVE_HIGH,
    };
}

bool hardware_map_role_valid(hardware_role_t role)
{
    return role >= HARDWARE_ROLE_FLOAT_INPUT && role < HARDWARE_ROLE_COUNT;
}

bool hardware_map_polarity_valid(hardware_relay_polarity_t polarity)
{
    return polarity == HARDWARE_RELAY_ACTIVE_HIGH ||
           polarity == HARDWARE_RELAY_ACTIVE_LOW;
}

bool hardware_map_timer_start_phase_valid(hardware_timer_start_phase_t phase)
{
    return phase == HARDWARE_TIMER_START_PHASE_ON ||
           phase == HARDWARE_TIMER_START_PHASE_OFF;
}

bool hardware_map_cooling_mode_valid(hardware_cooling_mode_t mode)
{
    return mode == HARDWARE_COOLING_MODE_AUTO ||
           mode == HARDWARE_COOLING_MODE_FORCE_OFF ||
           mode == HARDWARE_COOLING_MODE_TEST_ON;
}

const char *hardware_map_role_name(hardware_role_t role)
{
    switch (role) {
    case HARDWARE_ROLE_FLOAT_INPUT:
        return "float_input";
    case HARDWARE_ROLE_PUMP_RELAY_1:
        return "pump_relay_1";
    case HARDWARE_ROLE_PUMP_RELAY_2:
        return "pump_relay_2";
    case HARDWARE_ROLE_DS18B20_DATA:
        return "ds18b20_data";
    case HARDWARE_ROLE_COOLING_RELAY:
        return "cooling_relay";
    default:
        return "unknown";
    }
}

const char *hardware_map_polarity_name(hardware_relay_polarity_t polarity)
{
    switch (polarity) {
    case HARDWARE_RELAY_ACTIVE_HIGH:
        return "active_high";
    case HARDWARE_RELAY_ACTIVE_LOW:
        return "active_low";
    default:
        return "unknown";
    }
}

const char *hardware_map_timer_start_phase_name(hardware_timer_start_phase_t phase)
{
    switch (phase) {
    case HARDWARE_TIMER_START_PHASE_ON:
        return "on";
    case HARDWARE_TIMER_START_PHASE_OFF:
        return "off";
    default:
        return "unknown";
    }
}

const char *hardware_map_cooling_mode_name(hardware_cooling_mode_t mode)
{
    switch (mode) {
    case HARDWARE_COOLING_MODE_AUTO:
        return "auto";
    case HARDWARE_COOLING_MODE_FORCE_OFF:
        return "force_off";
    case HARDWARE_COOLING_MODE_TEST_ON:
        return "test_on";
    default:
        return "unknown";
    }
}

const hardware_gpio_option_t *hardware_map_options_for_role(hardware_role_t role,
                                                            size_t *count)
{
    if (count) {
        *count = 0;
    }

    switch (role) {
    case HARDWARE_ROLE_FLOAT_INPUT:
        if (count) {
            *count = ARRAY_SIZE(s_float_options);
        }
        return s_float_options;
    case HARDWARE_ROLE_PUMP_RELAY_1:
        if (count) {
            *count = ARRAY_SIZE(s_pump_relay1_options);
        }
        return s_pump_relay1_options;
    case HARDWARE_ROLE_PUMP_RELAY_2:
        if (count) {
            *count = ARRAY_SIZE(s_pump_relay2_options);
        }
        return s_pump_relay2_options;
    case HARDWARE_ROLE_DS18B20_DATA:
        if (count) {
            *count = ARRAY_SIZE(s_ds18b20_options);
        }
        return s_ds18b20_options;
    case HARDWARE_ROLE_COOLING_RELAY:
        if (count) {
            *count = ARRAY_SIZE(s_cooling_relay_options);
        }
        return s_cooling_relay_options;
    default:
        return NULL;
    }
}

bool hardware_map_gpio_allowed_for_role(hardware_role_t role, gpio_num_t gpio)
{
    size_t count = 0;
    const hardware_gpio_option_t *options = hardware_map_options_for_role(role, &count);
    if (!options) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (options[i].gpio == gpio) {
            return true;
        }
    }
    return false;
}

bool hardware_map_validate(const hardware_map_t *map)
{
    if (!map) {
        return false;
    }

    gpio_num_t gpios[HARDWARE_ROLE_COUNT] = {
        map->float_input_gpio,
        map->pump_relay1_gpio,
        map->pump_relay2_gpio,
        map->ds18b20_data_gpio,
        map->cooling_relay_gpio,
    };

    for (int role = HARDWARE_ROLE_FLOAT_INPUT; role < HARDWARE_ROLE_COUNT; role++) {
        if (!hardware_map_gpio_allowed_for_role((hardware_role_t)role, gpios[role])) {
            return false;
        }
        for (int other = role + 1; other < HARDWARE_ROLE_COUNT; other++) {
            if (gpios[role] == gpios[other]) {
                return false;
            }
        }
    }

    return true;
}

bool hardware_map_equal(const hardware_map_t *a, const hardware_map_t *b)
{
    return a && b &&
           a->float_input_gpio == b->float_input_gpio &&
           a->pump_relay1_gpio == b->pump_relay1_gpio &&
           a->pump_relay2_gpio == b->pump_relay2_gpio &&
           a->ds18b20_data_gpio == b->ds18b20_data_gpio &&
           a->cooling_relay_gpio == b->cooling_relay_gpio;
}

bool hardware_map_reboot_required(const hardware_map_t *active,
                                  const hardware_map_t *pending)
{
    return active && pending && !hardware_map_equal(active, pending);
}

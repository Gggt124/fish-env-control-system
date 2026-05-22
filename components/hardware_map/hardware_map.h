#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HARDWARE_ROLE_FLOAT_INPUT = 0,
    HARDWARE_ROLE_PUMP_RELAY_1,
    HARDWARE_ROLE_PUMP_RELAY_2,
    HARDWARE_ROLE_DS18B20_DATA,
    HARDWARE_ROLE_COOLING_RELAY,
    HARDWARE_ROLE_COUNT,
} hardware_role_t;

typedef enum {
    HARDWARE_RELAY_ACTIVE_HIGH = 0,
    HARDWARE_RELAY_ACTIVE_LOW,
} hardware_relay_polarity_t;

typedef enum {
    HARDWARE_TIMER_START_PHASE_ON = 0,
    HARDWARE_TIMER_START_PHASE_OFF,
} hardware_timer_start_phase_t;

typedef enum {
    HARDWARE_COOLING_MODE_AUTO = 0,
    HARDWARE_COOLING_MODE_FORCE_OFF,
    HARDWARE_COOLING_MODE_TEST_ON,
} hardware_cooling_mode_t;

typedef struct {
    gpio_num_t float_input_gpio;
    gpio_num_t pump_relay1_gpio;
    gpio_num_t pump_relay2_gpio;
    gpio_num_t ds18b20_data_gpio;
    gpio_num_t cooling_relay_gpio;
} hardware_map_t;

typedef struct {
    hardware_relay_polarity_t pump_relay1;
    hardware_relay_polarity_t pump_relay2;
    hardware_relay_polarity_t cooling_relay;
} hardware_relay_polarity_map_t;

typedef struct {
    hardware_role_t role;
    gpio_num_t gpio;
    bool input_capable;
    bool output_capable;
    bool internal_pull_capable;
    bool is_default;
    const char *label;
} hardware_gpio_option_t;

hardware_map_t hardware_map_defaults(void);
hardware_relay_polarity_map_t hardware_map_default_polarities(void);

const char *hardware_map_role_name(hardware_role_t role);
const char *hardware_map_polarity_name(hardware_relay_polarity_t polarity);
const char *hardware_map_timer_start_phase_name(hardware_timer_start_phase_t phase);
const char *hardware_map_cooling_mode_name(hardware_cooling_mode_t mode);

bool hardware_map_role_valid(hardware_role_t role);
bool hardware_map_polarity_valid(hardware_relay_polarity_t polarity);
bool hardware_map_timer_start_phase_valid(hardware_timer_start_phase_t phase);
bool hardware_map_cooling_mode_valid(hardware_cooling_mode_t mode);

const hardware_gpio_option_t *hardware_map_options_for_role(hardware_role_t role,
                                                            size_t *count);
bool hardware_map_gpio_allowed_for_role(hardware_role_t role, gpio_num_t gpio);
bool hardware_map_validate(const hardware_map_t *map);
bool hardware_map_equal(const hardware_map_t *a, const hardware_map_t *b);
bool hardware_map_reboot_required(const hardware_map_t *active,
                                  const hardware_map_t *pending);

#ifdef __cplusplus
}
#endif

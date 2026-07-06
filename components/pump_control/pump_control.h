#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PUMP_CONTROL_RELAY_ACTIVE_HIGH = 0,
    PUMP_CONTROL_RELAY_ACTIVE_LOW,
} pump_control_relay_polarity_t;

typedef enum {
    PUMP_CONTROL_FLOAT_UNKNOWN = 0,
    PUMP_CONTROL_FLOAT_OFF,
    PUMP_CONTROL_FLOAT_ON,
} pump_control_float_state_t;

typedef enum {
    PUMP_CONTROL_TIMER_NONE = 0,
    PUMP_CONTROL_TIMER_1,
    PUMP_CONTROL_TIMER_2,
} pump_control_active_timer_t;

typedef enum {
    PUMP_CONTROL_RELAY_NONE = 0,
    PUMP_CONTROL_RELAY_1,
    PUMP_CONTROL_RELAY_2,
} pump_control_active_relay_t;

typedef enum {
    PUMP_CONTROL_PHASE_IDLE = 0,
    PUMP_CONTROL_PHASE_ON,
    PUMP_CONTROL_PHASE_OFF,
} pump_control_timer_phase_t;

typedef enum {
    PUMP_CONTROL_START_PHASE_ON = 0,
    PUMP_CONTROL_START_PHASE_OFF,
} pump_control_start_phase_t;

typedef struct {
    uint32_t on_sec;
    uint32_t off_sec;
} pump_control_timer_config_t;

typedef struct {
    pump_control_timer_config_t   timer1;
    pump_control_timer_config_t   timer2;
    pump_control_start_phase_t    timer1_start_phase;
    pump_control_start_phase_t    timer2_start_phase;
    pump_control_relay_polarity_t relay1_polarity;
    pump_control_relay_polarity_t relay2_polarity;
} pump_control_timer_update_t;

typedef struct {
    gpio_num_t float_gpio;
    bool float_active_low;
    gpio_num_t relay_gpio;
    pump_control_relay_polarity_t relay_polarity;
    gpio_num_t relay1_gpio;
    gpio_num_t relay2_gpio;
    pump_control_relay_polarity_t relay1_polarity;
    pump_control_relay_polarity_t relay2_polarity;
    uint32_t debounce_ms;
    pump_control_timer_config_t timer1;
    pump_control_timer_config_t timer2;
    pump_control_start_phase_t timer1_start_phase;
    pump_control_start_phase_t timer2_start_phase;
} pump_control_config_t;

typedef struct {
    bool initialized;
    bool config_valid;
    bool running;
    bool initial_stabilizing;
    bool fault;
    bool relay_energized;
    bool relay1_energized;
    bool relay2_energized;
    gpio_num_t float_gpio;
    gpio_num_t relay_gpio;
    gpio_num_t relay1_gpio;
    gpio_num_t relay2_gpio;
    pump_control_float_state_t float_state;
    pump_control_active_timer_t active_timer;
    pump_control_active_relay_t active_relay;
    pump_control_timer_phase_t phase;
    uint32_t countdown_sec;
    uint32_t total_duration_sec;
} pump_control_status_t;

pump_control_config_t pump_control_default_config(void);
bool pump_control_init(const pump_control_config_t *config);
bool pump_control_start(void);
bool pump_control_stop(void);
bool pump_control_get_status(pump_control_status_t *out);
bool pump_control_update_timers(const pump_control_timer_update_t *update);

#ifdef __cplusplus
}
#endif

0058204 fix(pump_control): fix cooldown edge-trigger, debounce bounds, boot cooldown, and rounding off-by-one
f2e06fd feat(pump_control): add min_dwell cooldown gate after timer switch

 components/pump_control/pump_control.c | 52 ++++++++++++++++++++++++++++++----
 1 file changed, 47 insertions(+), 5 deletions(-)

diff --git a/components/pump_control/pump_control.c b/components/pump_control/pump_control.c
index cfe3851..d84a731 100644
--- a/components/pump_control/pump_control.c
+++ b/components/pump_control/pump_control.c
@@ -29,20 +29,21 @@ static bool s_relay2_energized;
 static pump_control_float_state_t s_confirmed_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
 static pump_control_float_state_t s_pending_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
 static int64_t s_pending_since_ms;
 
 static pump_control_active_timer_t s_active_timer = PUMP_CONTROL_TIMER_NONE;
 static pump_control_active_relay_t s_active_relay = PUMP_CONTROL_RELAY_NONE;
 static pump_control_timer_phase_t s_phase = PUMP_CONTROL_PHASE_IDLE;
 static uint32_t s_countdown_sec;
 static int64_t s_phase_deadline_ms;
 static uint32_t s_current_phase_duration_sec;
+static int64_t s_last_switch_ms;   /* timestamp of the most recent Timer1<->Timer2 switch */
 
 
 
 static bool polarity_valid(pump_control_relay_polarity_t polarity)
 {
     return polarity == PUMP_CONTROL_RELAY_ACTIVE_HIGH ||
            polarity == PUMP_CONTROL_RELAY_ACTIVE_LOW;
 }
 
 static bool start_phase_valid(pump_control_start_phase_t phase)
@@ -94,21 +95,23 @@ static bool config_valid(const pump_control_config_t *config)
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
-        config->debounce_ms == 0) {
+        config->debounce_ms == 0 ||
+        config->debounce_ms > 60000 ||
+        config->min_dwell_sec > APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC) {
         return false;
     }
     return timer_config_valid(&config->timer1) &&
            timer_config_valid(&config->timer2);
 }
 
 static void reset_runtime_state_locked(void)
 {
     s_running = false;
     s_initial_stabilizing = false;
@@ -118,20 +121,21 @@ static void reset_runtime_state_locked(void)
     s_relay2_energized = false;
     s_confirmed_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
     s_pending_float_state = PUMP_CONTROL_FLOAT_UNKNOWN;
     s_pending_since_ms = 0;
     s_active_timer = PUMP_CONTROL_TIMER_NONE;
     s_active_relay = PUMP_CONTROL_RELAY_NONE;
     s_phase = PUMP_CONTROL_PHASE_IDLE;
     s_countdown_sec = 0;
     s_phase_deadline_ms = 0;
     s_current_phase_duration_sec = 0;
+    s_last_switch_ms = 0;
 }
 
 static gpio_num_t relay_gpio_for_config(const pump_control_config_t *config,
                                         pump_control_active_relay_t relay)
 {
     if (!config) {
         return GPIO_NUM_NC;
     }
     if (relay == PUMP_CONTROL_RELAY_1) {
         return config->relay1_gpio;
@@ -189,20 +193,21 @@ static void mark_fault_locked(void)
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
+    s_last_switch_ms = 0;
     force_both_relays_inactive_for_config(&s_config);
 }
 
 static void force_both_relays_inactive_locked(void)
 {
     s_relay_energized = false;
     s_relay1_energized = false;
     s_relay2_energized = false;
     if (!s_relay_configured) {
         return;
@@ -391,29 +396,34 @@ static void preview_channel_for_float_locked(pump_control_float_state_t float_st
     force_both_relays_inactive_locked();
 }
 
 static void start_channel_for_float_locked(pump_control_float_state_t float_state, int64_t now_ms)
 {
     force_both_relays_inactive_locked();
     if (s_fault) {
         return;
     }
 
+    pump_control_active_timer_t prev_timer = s_active_timer;
     set_selected_channel_for_float_locked(float_state);
     if (s_active_timer == PUMP_CONTROL_TIMER_NONE) {
         s_phase = PUMP_CONTROL_PHASE_IDLE;
         s_countdown_sec = 0;
         s_phase_deadline_ms = 0;
         force_both_relays_inactive_locked();
         return;
     }
 
+    if (s_active_timer != prev_timer && prev_timer != PUMP_CONTROL_TIMER_NONE) {
+        s_last_switch_ms = now_ms;
+    }
+
     set_phase_locked(start_phase_to_timer_phase(active_timer_start_phase_locked()), now_ms);
 }
 
 static bool update_debounce_locked(pump_control_float_state_t raw_state, int64_t now_ms)
 {
     if (s_pending_float_state != raw_state) {
         s_pending_float_state = raw_state;
         s_pending_since_ms = now_ms;
         return false;
     }
@@ -452,40 +462,56 @@ static void pump_tick_cb(void *arg)
         return;
     }
 
     if (!s_initialized || !s_config_valid || s_fault) {
         xSemaphoreGive(s_pump_mutex);
         return;
     }
 
     int64_t now_ms = esp_timer_get_time() / 1000;
     pump_control_float_state_t raw_state = read_float_state_locked();
-    bool float_changed = update_debounce_locked(raw_state, now_ms);
+    update_debounce_locked(raw_state, now_ms);
 
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
 
-    if (float_changed) {
-        start_channel_for_float_locked(s_confirmed_float_state, now_ms);
+    pump_control_active_timer_t required_timer = PUMP_CONTROL_TIMER_NONE;
+    if (s_confirmed_float_state == PUMP_CONTROL_FLOAT_ON) {
+        required_timer = PUMP_CONTROL_TIMER_2;
+    } else if (s_confirmed_float_state == PUMP_CONTROL_FLOAT_OFF) {
+        required_timer = PUMP_CONTROL_TIMER_1;
+    }
+
+    bool timer_switch_needed = (required_timer != s_active_timer && required_timer != PUMP_CONTROL_TIMER_NONE);
+
+    if (timer_switch_needed) {
+        bool cooldown_active =
+            s_config.min_dwell_sec > 0 &&
+            (now_ms - s_last_switch_ms) < ((int64_t)s_config.min_dwell_sec * 1000);
+        if (cooldown_active) {
+            advance_phase_if_needed_locked(now_ms);
+        } else {
+            start_channel_for_float_locked(s_confirmed_float_state, now_ms);
+        }
     } else {
         advance_phase_if_needed_locked(now_ms);
     }
 
     s_countdown_sec = seconds_remaining(now_ms);
     xSemaphoreGive(s_pump_mutex);
 }
 
 static bool ensure_timer_locked(void)
 {
@@ -511,20 +537,21 @@ pump_control_config_t pump_control_default_config(void)
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
+        .min_dwell_sec = APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC,
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
@@ -675,21 +702,24 @@ bool pump_control_update_timers(const pump_control_timer_update_t *update)
         ESP_LOGE(TAG, "pump_control not initialized");
         return false;
     }
 
     /* Validate all fields before taking the lock */
     if (!timer_config_valid(&update->timer1) ||
         !timer_config_valid(&update->timer2) ||
         !polarity_valid(update->relay1_polarity) ||
         !polarity_valid(update->relay2_polarity) ||
         !start_phase_valid(update->timer1_start_phase) ||
-        !start_phase_valid(update->timer2_start_phase)) {
+        !start_phase_valid(update->timer2_start_phase) ||
+        update->debounce_ms == 0 ||
+        update->debounce_ms > 60000 ||
+        update->min_dwell_sec > APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC) {
         ESP_LOGE(TAG, "pump_control_update_timers: invalid update fields");
         return false;
     }
 
     if (xSemaphoreTake(s_pump_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
         return false;
     }
 
     if (!s_initialized || !s_config_valid || s_fault) {
         xSemaphoreGive(s_pump_mutex);
@@ -697,20 +727,22 @@ bool pump_control_update_timers(const pump_control_timer_update_t *update)
     }
 
     /* Apply soft config in-place — GPIO, relay state, and phase are untouched */
     s_config.timer1             = update->timer1;
     s_config.timer2             = update->timer2;
     s_config.timer1_start_phase = update->timer1_start_phase;
     s_config.timer2_start_phase = update->timer2_start_phase;
     s_config.relay1_polarity    = update->relay1_polarity;
     s_config.relay2_polarity    = update->relay2_polarity;
     s_config.relay_polarity     = update->relay1_polarity; /* legacy alias */
+    s_config.debounce_ms        = update->debounce_ms;
+    s_config.min_dwell_sec      = update->min_dwell_sec;
 
     if (s_running && s_phase != PUMP_CONTROL_PHASE_IDLE) {
         set_active_relay_energized_locked(s_phase == PUMP_CONTROL_PHASE_ON);
     }
 
     ESP_LOGI(TAG, "Timer config updated live: t1=%lus/%lus t2=%lus/%lus",
              (unsigned long)update->timer1.on_sec,
              (unsigned long)update->timer1.off_sec,
              (unsigned long)update->timer2.on_sec,
              (unsigned long)update->timer2.off_sec);
@@ -751,14 +783,24 @@ bool pump_control_get_status(pump_control_status_t *out)
     out->relay_gpio = active_relay_gpio_locked();
     out->relay1_gpio = s_config.relay1_gpio;
     out->relay2_gpio = s_config.relay2_gpio;
     out->float_state = s_confirmed_float_state;
     out->active_timer = s_active_timer;
     out->active_relay = s_active_relay;
     out->phase = s_phase;
     out->countdown_sec = s_countdown_sec;
     out->total_duration_sec = (s_phase == PUMP_CONTROL_PHASE_ON || s_phase == PUMP_CONTROL_PHASE_OFF) 
                               ? s_current_phase_duration_sec : 0;
+    out->min_dwell_sec = s_config.min_dwell_sec;
+    if (s_running && s_config.min_dwell_sec > 0 && s_last_switch_ms > 0) {
+        int64_t now_ms_local = esp_timer_get_time() / 1000;
+        int64_t elapsed_ms = now_ms_local - s_last_switch_ms;
+        int64_t cooldown_ms = (int64_t)s_config.min_dwell_sec * 1000;
+        int64_t remaining_ms = cooldown_ms - elapsed_ms;
+        out->cooldown_remaining_sec = remaining_ms > 0 ? (uint32_t)((remaining_ms + 999) / 1000) : 0;
+    } else {
+        out->cooldown_remaining_sec = 0;
+    }
 
     xSemaphoreGive(s_pump_mutex);
     return true;
 }

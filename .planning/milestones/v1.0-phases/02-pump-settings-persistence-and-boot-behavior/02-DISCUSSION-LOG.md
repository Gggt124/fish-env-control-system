# Phase 2: Pump Settings Persistence And Boot Behavior - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-05-19
**Phase:** 2-Pump Settings Persistence And Boot Behavior
**Areas discussed:** Boot auto-start timing, Invalid saved settings, Settings ownership, Future save behavior

---

## Boot Auto-Start Timing

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| When should pump control auto-start after boot? | After config load | Load NVS, validate config, initialize pump control with loaded values, then start immediately; relay remains inactive during init and starts only when config is valid | yes |
| When should pump control auto-start after boot? | After web ready | Wait for Wi-Fi/web server before starting | |
| When should pump control auto-start after boot? | Manual first boot | Start only after user has saved config at least once | |
| What if auto-start init/start fails? | Fail-safe and continue web | Relay inactive, pump stopped, log error, Wi-Fi/web setup continues so user can fix config later | yes |
| What if auto-start init/start fails? | Stop boot | Return from app_main() on pump init/start failure | |
| What if auto-start init/start fails? | Retry in loop | Boot web and retry start periodically | |
| How should Phase 2 change boot order? | NVS first, then pump init | Initialize NVS, load pump settings, initialize pump with loaded values, then auto-start if enabled | yes |
| How should Phase 2 change boot order? | Safe default init first, then re-init | Initialize with safe defaults before NVS, then re-initialize with loaded values | |
| How should Phase 2 change boot order? | Keep current order for now | Keep default init before NVS and defer true reboot behavior | |
| What if saved auto-start is disabled? | Init but stopped | Load config, validate, initialize GPIO/relay inactive, leave running=false | yes |
| What if saved auto-start is disabled? | Do not init pump | Do not initialize pump component until user starts it | |
| What if saved auto-start is disabled? | Init defaults only | Initialize relay inactive with default config but do not use saved timer settings until start | |

**User's choice:** After config load; fail-safe and continue web; NVS first then pump init; init but stopped when auto-start is disabled.
**Notes:** User selected the recommended safety-preserving boot path for every boot question.

---

## Invalid Saved Settings

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| What if saved pump settings are invalid? | Fallback defaults, no auto-start | Use safe default config, initialize pump stopped, log warning, do not auto-start for that boot | yes |
| What if saved pump settings are invalid? | Fallback defaults, auto-start | Use default config and auto-start if saved auto-start was enabled | |
| What if saved pump settings are invalid? | Reject and leave uninitialized | Do not initialize pump component when config is invalid | |
| Should invalid settings be overwritten with defaults? | Do not overwrite automatically | Preserve bad saved data for logs/status and let future API/UI save explicitly | yes |
| Should invalid settings be overwritten with defaults? | Overwrite with defaults | Write default values back to NVS immediately | |
| Should invalid settings be overwritten with defaults? | Clear pump namespace | Erase pump settings and load defaults later | |
| What if no pump settings exist? | Clean first boot | Not an error; use defaults and default auto-start enabled | yes |
| What if no pump settings exist? | Missing config warning | Use defaults but flag that settings have never been saved | |
| What if no pump settings exist? | Require explicit save | Use defaults but do not auto-start until config has been saved | |
| Should load result distinguish missing and invalid? | Track load status | Distinguish defaults_used_missing, defaults_used_invalid, loaded_saved, or equivalent | yes |
| Should load result distinguish missing and invalid? | Boolean only | Track only loaded/fallback | |
| Should load result distinguish missing and invalid? | No status tracking | Log only | |

**User's choice:** Fallback defaults without auto-start for invalid saved settings; do not overwrite invalid NVS automatically; treat missing settings as clean first boot; track load status.
**Notes:** Invalid saved settings are treated differently from missing settings because invalid saved data may reflect prior user intent or corruption and should not silently energize the pump.

---

## Settings Ownership

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| Which values should Phase 2 persist? | Timers + relay polarity + auto-start | Persist Timer 1/2 ON/OFF, relay polarity, and auto-start; keep GPIO and debounce as source defaults | yes |
| Which values should Phase 2 persist? | Timers + auto-start only | Persist only minimum requirement fields | |
| Which values should Phase 2 persist? | All pump config | Persist GPIO, float polarity, and debounce too | |
| Which component owns persistence? | Extend nvs_store | Add save/load pump settings APIs alongside existing Wi-Fi/AP config persistence | yes |
| Which component owns persistence? | New pump_settings component | Add a separate pump settings component | |
| Which component owns persistence? | Inside pump_control | Let pump_control read/write NVS directly | |
| What NVS namespace should be used? | Separate namespace pump_cfg | Keep pump settings out of wifi_cfg | yes |
| What NVS namespace should be used? | Reuse wifi_cfg | Store pump keys in the existing Wi-Fi namespace | |
| What NVS namespace should be used? | Single app namespace later | Move all config to a future shared namespace | |
| What save/load struct shape should be used? | Dedicated persisted struct | Persisted fields only, then merge with pump_control_default_config() at boot | yes |
| What save/load struct shape should be used? | Save full pump_control_config_t | Save runtime config directly | |
| What save/load struct shape should be used? | Individual parameters only | Use separate function parameters for every setting | |

**User's choice:** Persist Timer 1/2 ON/OFF, relay polarity, and auto-start; extend `nvs_store`; use `pump_cfg`; use a dedicated persisted struct.
**Notes:** GPIO, float polarity, and debounce remain source defaults because editable hardware config belongs to later v2 requirements.

---

## Future Save Behavior

| Question | Option | Description | Selected |
|----------|--------|-------------|----------|
| How should future API save apply while running? | Save then restart runtime safely | Validate, save NVS, stop relay inactive, re-init with new config, then restart if previously running | yes |
| How should future API save apply while running? | Save only, next start/boot applies | Persist only and defer runtime apply | |
| How should future API save apply while running? | Hot apply without stop | Change config without stopping/restarting timer state | |
| Does disabling auto-start stop a running pump? | Auto-start affects boot only | Changing auto_start does not stop a currently running pump | yes |
| Does disabling auto-start stop a running pump? | Disable means stop now | Treat auto-start disabled as immediate runtime stop | |
| Does disabling auto-start stop a running pump? | Ask UI to separate controls later | Leave semantics undecided | |
| How should future API save apply while stopped? | Re-init stopped with new config | Validate, save, re-init pump with new config, keep stopped/relay inactive | yes |
| How should future API save apply while stopped? | Save only | Defer runtime apply until next start/boot | |
| How should future API save apply while stopped? | Start if auto-start enabled | Start after save when auto-start is enabled | |
| What if NVS save fails? | Do not change runtime on save failure | Keep old runtime config/state and return API error | yes |
| What if NVS save fails? | Apply runtime anyway | Apply runtime config even though persistence failed | |
| What if NVS save fails? | Stop pump on save failure | Stop pump when save fails | |
| Should Phase 2 add factory reset? | Helper only, no route | Optional clear/default helper for tests/future API, no HTTP route/UI in Phase 2 | yes |
| Should Phase 2 add factory reset? | No clear helper yet | Implement only load/save | |
| Should Phase 2 add factory reset? | Add API route now | Add an HTTP route for clearing pump settings | |

**User's choice:** Save succeeds before runtime mutation; running save restarts safely; stopped save re-initializes stopped; auto-start is boot-only; optional helper only, no route.
**Notes:** These choices are captured now so Phase 3 API planning does not reinterpret runtime apply semantics.

---

## the agent's Discretion

- Exact function names, enum names, NVS key names, and load-status type names may be chosen during planning.
- Planner may decide whether the optional clear/default helper is useful in Phase 2.

## Deferred Ideas

None.

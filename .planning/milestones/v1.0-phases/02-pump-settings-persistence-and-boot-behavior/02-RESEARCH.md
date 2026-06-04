# Phase 02 Research: Pump Settings Persistence And Boot Behavior

## RESEARCH COMPLETE

## Scope

Phase 2 adds persistent pump settings and changes boot behavior so `pump_control` is initialized from resolved settings after NVS is available. It does not add web APIs, UI controls, GPIO configurability, debounce configurability, factory reset routes, or hardware validation.

## Current Code Findings

- `main/app_main.c` currently initializes `pump_control` before `nvs_store_init()`, using only `pump_control_default_config()`, and never calls `pump_control_start()`.
- `components/nvs_store/` already owns NVS initialization and Wi-Fi/AP/static-IP persistence. It hides raw key names and returns simple boolean outcomes.
- `components/pump_control/` already validates timer duration bounds, relay polarity, GPIOs, and debounce in `pump_control_init()`. It keeps runtime state private and should not depend on NVS.
- `components/app_config/app_config.h` already contains timer defaults and relay polarity defaults, but does not yet contain an explicit auto-start default constant.
- `components/nvs_store/CMakeLists.txt` currently requires only `app_config` and `nvs_flash`, which is enough if persisted pump settings use narrow storage types instead of `pump_control` types.

## Recommended Approach

Use `nvs_store` as the persistence boundary and keep `pump_control` storage-agnostic.

Add a storage-only struct in `nvs_store.h` with:

- Timer 1 ON seconds
- Timer 1 OFF seconds
- Timer 2 ON seconds
- Timer 2 OFF seconds
- Relay active-low boolean
- Auto-start boolean

Add a load-status enum with at least:

- `NVS_STORE_PUMP_SETTINGS_LOADED`
- `NVS_STORE_PUMP_SETTINGS_DEFAULTS_MISSING`
- `NVS_STORE_PUMP_SETTINGS_DEFAULTS_INVALID`

Treat a missing namespace or absent pump keys as first boot and return defaults with auto-start enabled. Treat partially present, out-of-range, or invalid stored values as invalid saved settings: return product defaults, report invalid status, and do not erase or repair NVS.

At boot, reorder `app_main()` so:

1. NVS initializes first.
2. Pump settings load into the storage struct.
3. The storage struct merges into `pump_control_default_config()`.
4. `pump_control_init()` runs with the resolved config.
5. `pump_control_start()` runs only if the resolved auto-start is enabled and the load status was not invalid.

If init or start fails, log the error, keep the relay inactive/stopped through the existing `pump_control` safety paths, and continue Wi-Fi/web startup.

## Key Risks

- Adding `pump_control` types directly to `nvs_store.h` would make the storage component depend on runtime GPIO code and may create avoidable coupling. Use storage-native booleans and seconds instead.
- Saving invalid settings should not partly commit. Validate all fields before opening NVS for write or before calling `nvs_commit()`.
- A missing single key after some keys exist should be treated as invalid saved settings, not first boot, because it indicates partial/corrupt saved data.
- Auto-start must be suppressed only for invalid saved settings. Missing settings should use defaults and auto-start enabled.
- `nvs_commit()` return values should be checked. Existing Wi-Fi helpers do not always include commit failure in `ok`; the pump save helper should because future API behavior depends on save-before-runtime-apply semantics.

## Validation Strategy

- Static checks with `rg` for new API names, namespace/key names, default constants, boot ordering, and `pump_control_start()`.
- Build validation with `.\scripts\build.ps1` or the documented `idf.py build` workflow.
- No flash/hardware validation in this phase. Reboot and live NVS behavior are manually validated in Phase 5.

## References Used

- Local ESP-IDF project code only.
- No external repositories were used, so `REFERENCE.md` does not need an update for this phase.


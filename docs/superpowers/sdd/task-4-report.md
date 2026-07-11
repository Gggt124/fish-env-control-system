## Task 4 Report: nvs_store min_dwell_sec persistence

### Implementation
- Appended `min_dwell_sec` to `nvs_store_pump_settings_t` in `components/nvs_store/nvs_store.h`.
- Bumped `NVS_PUMP_BLOB_VERSION` to 2 in `components/nvs_store/nvs_store.c`.
- Updated `nvs_store_load_pump_settings` to accept both version 1 (legacy) and version 2 (new).
- Set `min_dwell_sec` default to `APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_SEC` in `nvs_store_pump_settings_defaults`.
- Added upper limit validation `<= APP_TEMPLATE_PUMP_FLOAT_MIN_DWELL_MAX_SEC` in `pump_settings_valid`.

### Testing
- `.\scripts\build.ps1 -Target esp32` completed successfully.
- `.\scripts\build.ps1 -Target esp32s3` completed successfully.

### Files Changed
- `components/nvs_store/nvs_store.h`
- `components/nvs_store/nvs_store.c`

### Self-Review
- Verified that `min_dwell_sec` is at the end of the structure (critical for version compatibility).
- Tested both ESP32 and ESP32S3 profiles; build outputs are pristine.
- Implemented the fallback handling so v1 configs are not discarded on load.

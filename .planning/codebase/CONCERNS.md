# Codebase Concerns

**Analysis Date:** 2026-06-14

## Tech Debt

**Default hardcoded credentials:**
- Issue: Default username `admin` and password `admin123` are hardcoded in configuration headers.
- File: [app_config.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/app_config/app_config.h#L33-L34)
- Why: Simple template fallback for initial startup and factory resets.
- Impact: Highly vulnerable if deployed without custom credentials configuration.
- Fix approach: Prompt the user to set a custom password on the first login and save it to NVS.

**Boot button sharing constraints:**
- Issue: The BOOT button is mapped to GPIO 0, which is also the ESP32 strapping pin.
- File: [app_config.h](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/app_config/app_config.h#L102)
- Why: Reusing the on-board boot button saves GPIOs.
- Impact: If held down during power-up, it forces the board into ROM download mode instead of booting the application.
- Fix approach: The application implements a veto logic in [app_main.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/app_main.c#L523-L528) to ignore boot-time presses, but physical holds during resets still interrupt normal boot.

## Known Bugs

- **None currently identified:** All features build and pass static style tests.

## Security Considerations

**Non-HttpOnly Session Cookie:**
- Risk: Session cookie is readable by frontend JS (`document.cookie.session`) to perform client-side redirection checks.
- File: [web_server.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/web_server.c)
- Current mitigation: The device is a local-only appliance with no internet gateway or third-party JS scripts, reducing XSS vector risk.
- Recommendations: Set HttpOnly flag on cookies and rely on API responses (`401 Unauthorized`) for dashboard redirect triggers.

**Plaintext credentials in flash memory:**
- Risk: Wi-Fi passwords and logins are saved in plaintext within the NVS partition.
- File: [nvs_store.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/nvs_store/nvs_store.c)
- Current mitigation: Flash encryption is disabled by default for development.
- Recommendations: Enable ESP32 NVS encryption and Flash encryption in production settings (`sdkconfig`).

## Performance Bottlenecks

**Blocking Wi-Fi scan requests:**
- Problem: Calling Wi-Fi scan blocks the calling web server thread while compiling scan tables.
- File: [wifi_manager.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/wifi_manager/wifi_manager.c)
- Measurement: Scans block HTTP handlers for 1.5s to 3s.
- Cause: Synchronous wait using binary semaphores inside API endpoints.
- Improvement path: Make Wi-Fi scans asynchronous; return immediately and poll the results table once ready.

**Chunked static file reads:**
- Problem: Static files are read and served from flash in 2048-byte chunks.
- File: [web_server.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/web_server.c)
- Measurement: Serving large JS/CSS files adds ~100ms request latency.
- Cause: Conserves heap memory by loading small chunks iteratively.
- Improvement path: Compress static assets using gzip before embedding to reduce flash reads and network payloads.

## Fragile Areas

**EMBED_FILES symbol generation:**
- File: [CMakeLists.txt](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/main/CMakeLists.txt#L9-L12)
- Why fragile: CMake strips directory prefixes from embedded files. If path layouts change (e.g. moving `static/index.html` to `static/html/index.html`), the compiler symbols (`_binary_index_html_start`) remain the same, but renaming files causes linker errors if assembly definitions are not aligned.
- Safe modification: Check generated assembly symbols using `Get-Content build\login.html.S` when changing file structures.

**Virtual GPIO mapping conflicts:**
- File: [hardware_map.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/hardware_map/hardware_map.c)
- Why fragile: The system allows changing GPIO pins dynamically. If two roles (e.g., pump relay and float switch) are mapped to the same pin, it can cause hardware shorts, high-current draw, or loop failures.
- Safe modification: Pin validation logic in [hardware_map_validate()](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/hardware_map/hardware_map.c) rejects overlapping assignments, but wrong polarity configs can still energize pump modules unexpectedly.

## Scaling Limits

**Session token slot limits:**
- Current capacity: 4 concurrent active sessions.
- Limit: 5th login attempts will overwrite/deny sessions depending on slot availability.
- Symptoms: Active sessions get logged out unexpectedly when multiple users/devices connect.
- Scaling path: Increase session count macro in [session.c](file:///c:/Users/Copter/OneDrive/Desktop/fish-pump-RelayTimerControl/components/session/session.c).

**Wi-Fi scan limits:**
- Current capacity: Max 20 networks.
- Limit: Networks with lower RSSI values are dropped from the scan list if more than 20 APs are present.
- Symptoms: Nearby routers may fail to appear in the scan list.
- Scaling path: Increase `WIFI_SCAN_MAX` in `app_config.h`.

---

*Concerns audit: 2026-06-14*
*Update as issues are fixed or new ones discovered*

# Using This Repository As A Template

Use this checklist after cloning the repo into a new ESP32 project.

## 1. Rename The Project

- Update `project(main_dashboard_mcu)` in root `CMakeLists.txt`.
- Rename the repository folder if needed.
- Update product naming in `components/app_config/app_config.h`.

## 2. Customize Template Config

Edit `components/app_config/app_config.h`:

```c
#define APP_TEMPLATE_NAME               "My Product"
#define APP_TEMPLATE_FIRMWARE_VERSION   "v0.1.6"
#define APP_TEMPLATE_AP_SSID            "MyProduct-Setup"
#define APP_TEMPLATE_MDNS_HOSTNAME      "myproduct"
#define APP_TEMPLATE_DEFAULT_USERNAME   "admin"
#define APP_TEMPLATE_DEFAULT_PASSWORD   "change-me"
```

Keep `APP_TEMPLATE_AP_SSID` at 32 bytes or fewer.
Keep `APP_TEMPLATE_MDNS_HOSTNAME` lowercase ASCII with no spaces.

## 3. Keep Core Modules Stable

These modules are intended to be reused mostly as-is:

- `components/nvs_store`
- `components/session`
- `components/wifi_manager`
- `main/dns_server.c`
- login/auth helper logic in `main/web_server.c`

Prefer adding product behavior beside them instead of rewriting them.

## 4. Add Product Pages

Add HTML/CSS/JS under `main/static/`, then add the file to `EMBED_FILES` in
`main/CMakeLists.txt`.

Remember: ESP-IDF embedded file symbols strip the directory prefix.

For `main/static/device.html`, use:

```c
extern const uint8_t _binary_device_html_start[] asm("_binary_device_html_start");
extern const uint8_t _binary_device_html_end[]   asm("_binary_device_html_end");
```

## 5. Add Product API Routes

Add handlers in `main/web_server.c` and register them in `web_server_start()`.
Increase `APP_TEMPLATE_HTTP_MAX_URI_HANDLERS` in `app_config.h` if the route
count grows.

Use `require_auth(req)` for private routes and `is_same_origin(req)` for POST
routes that mutate state.

## 6. Build

```powershell
.\scripts\build.ps1
```

## 7. Manual Hardware Check

- AP appears with the configured SSID.
- `http://192.168.4.1` loads.
- Login works.
- `/api/status` returns JSON.
- Wi-Fi scan works.
- STA connect works.
- Saved STA config survives reboot.
- AP fallback works when STA fails.

## Do Not Add To This Template Core

Keep these in downstream products unless the template scope changes:

- OTA
- relay/GPIO control
- sensors
- MQTT/cloud
- WebSocket
- SPIFFS/LittleFS
- multi-user management
- production security hardening

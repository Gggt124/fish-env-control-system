# Reference Repositories

Repos and frameworks studied for best-practice adaptation into this project.

## ESP-IDF Official Examples

- **restful_server** — https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/restful_server
  - cJSON for all JSON construction/parsing (no hand-built strings)
  - `ESP_RETURN_ON_FALSE` / `ESP_GOTO_ON_ERROR` error propagation
  - Chunked file serving with proper MIME type detection
  - Versioned API endpoints (`/api/v1/...`)
  - Independent frontend/backend dev workflow (Vue.js + Vite)
  - mDNS for hostname resolution

- **file_serving** — https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/file_serving
  - HTTP file upload/download patterns
  - Chunked HTTP response from filesystem

- **captive_portal** — https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/captive_portal
  - DNS server captive portal pattern
  - SoftAP fallback UX

## Community Libraries

- **ESPAsyncWebServer** — https://github.com/ESP32Async/ESPAsyncWebServer (592 stars)
  - Async non-blocking HTTP, WebSocket, SSE, middleware, regex routing
  - Arduino/PlatformIO only (not ESP-IDF native)
  - Adapted concept: middleware pattern, async handler inspiration

- **WiFiManager** — https://github.com/tzapu/WiFiManager (7,200 stars)
  - Wi-Fi connection manager with captive portal fallback
  - Auto-AP fallback, configurable timeout, custom parameters, callbacks
  - Arduino library for ESP8266/ESP32
  - Adapted concept: session timeout, captive portal UX flow, auto-reconnect pattern

## Community ESP-IDF Wi-Fi Managers

- **tonyp7/esp32-wifi-manager** — https://github.com/tonyp7/esp32-wifi-manager
  - SoftAP auto-stop after STA connects (60s default via FreeRTOS timer)
  - AP restore on STA disconnect as fallback
  - NVS-based config for timeouts
  - Adapted concept: `esp_wifi_set_mode(WIFI_MODE_STA)` to drop AP at runtime, `esp_wifi_set_mode(WIFI_MODE_APSTA)` to restore, one-shot timer in `IP_EVENT_STA_GOT_IP` handler

- **hliebscher/esp_wifi_manager** — https://github.com/hliebscher/esp_wifi_manager
  - Configurable provisioning tear-down delay (5s default)
  - Retry limit before restoring AP fallback

## ESP-IDF Built-in

- **cJSON** — `components/json/cJSON/` in ESP-IDF
  - `cJSON_CreateObject()`, `cJSON_AddStringToObject()`, `cJSON_AddNumberToObject()`
  - `cJSON_Parse()`, `cJSON_Print()`, `cJSON_PrintUnformatted()`, `cJSON_Delete()`
  - Eliminates manual JSON string construction bugs (SSID escaping, etc.)

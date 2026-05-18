---
generated: 2026-05-18
focus: tech
---

# Integrations

## Summary

This firmware has only local-device integrations. It does not call cloud APIs, databases, MQTT brokers, webhooks, OTA services, or external HTTP services. The main integrations are ESP-IDF subsystems, the local browser UI, mDNS, captive DNS, Wi-Fi AP/STA networking, and NVS storage.

## Browser And HTTP UI

- `main/web_server.c` starts an ESP-IDF HTTP server with `HTTPD_DEFAULT_CONFIG()`.
- Static pages are embedded from `main/static/` by `main/CMakeLists.txt` and served from route handlers in `main/web_server.c`.
- `main/static/app.js` talks to device-local APIs with JSON over HTTP.
- Core pages are `/login`, `/dashboard`, `/status`, and `/wifi`.
- API routes are `/api/login`, `/api/logout`, `/api/status`, `/api/wifi/scan`, `/api/wifi/connect`, and `/api/wifi/disconnect`.

## Wi-Fi Network Integration

- `components/wifi_manager/wifi_manager.c` owns AP+STA initialization through ESP-IDF Wi-Fi APIs.
- The firmware boots in APSTA mode and starts SoftAP immediately.
- Default SoftAP settings come from `components/app_config/app_config.h`: SSID `FishPump-Setup`, open authentication, channel 1, and max 4 clients.
- Station credentials are loaded from NVS at boot and applied before `esp_wifi_start()`.
- Station connect requests come from `POST /api/wifi/connect` in `main/web_server.c`, which calls `wifi_manager_connect_sta()`.
- Scans are launched by `wifi_manager_scan()` and returned through `/api/wifi/scan`.

## NVS Storage

- `components/nvs_store/nvs_store.c` wraps ESP-IDF NVS APIs.
- Namespace: `wifi_cfg`.
- Stored Wi-Fi keys: `sta_ssid` and `sta_pass`.
- Stored static IP keys: `sta_ip`, `sta_gw`, `sta_netmask`, and `sta_dns`.
- Stored AP behavior keys: `ap_stop_tmo` and `ap_auto_stop`.
- NVS is initialized from `app_main()` through `nvs_store_init()`.
- Development config disables NVS encryption in `sdkconfig.defaults`.

## mDNS

- `main/app_main.c` initializes mDNS through the managed `espressif/mdns` component.
- Hostname comes from `APP_TEMPLATE_MDNS_HOSTNAME` in `components/app_config/app_config.h`.
- Current host is `fish-pump`, exposed as `http://fish-pump.local`.
- HTTP service is registered with `_http._tcp` on port 80.

## Captive DNS

- `main/dns_server.c` creates a UDP socket on port 53 with LwIP sockets.
- All valid DNS queries receive an A-record answer for `192.168.4.1`.
- The DNS task is started from `main/app_main.c` after HTTP and mDNS setup.
- Runtime status is exposed through `dns_server_is_running()` and included in `/api/status`.

## Session And Authentication

- `components/session/session.c` provides in-memory login sessions.
- `main/web_server.c` creates sessions after validating compile-time credentials from `components/app_config/app_config.h`.
- Session cookies are `session=<token>; Path=/; SameSite=Lax`.
- Cookies are intentionally readable by JavaScript for local prototype redirect logic.
- Sessions are not persisted to NVS and reset on reboot.

## Hardware Integration

- Current hardware target is classic ESP32.
- No relay GPIO, sensors, pump control, timers, or physical outputs are integrated in this phase.
- `README.md`, `AGENTS.md`, and `main/static/dashboard.html` all preserve this boundary until exact board, pin map, and timing requirements are known.

## External Services Not Present

- No cloud service integration.
- No MQTT.
- No OTA update service.
- No remote database.
- No HTTPS certificate integration.
- No WebSocket.
- No filesystem such as SPIFFS or LittleFS.


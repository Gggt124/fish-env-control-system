---
generated: 2026-05-18
refreshed: 2026-06-02
last_mapped_commit: 3ef2ac063ebead27920e5e340508c371f689cdca
focus: tech
scope: .gitignore,.graphify_detect.json,.planning,PRODUCT.md
---

# Integrations

## Summary

This firmware has only local-device integrations. It does not call cloud APIs, databases, MQTT brokers, webhooks, OTA services, or external HTTP services. The main integrations are ESP-IDF subsystems, the local browser UI, mDNS, captive DNS, Wi-Fi AP/STA networking, and NVS storage.

## Browser And HTTP UI

- `main/web_server.c` starts an ESP-IDF HTTP server with `HTTPD_DEFAULT_CONFIG()`.
- Static pages are embedded from `main/static/` by `main/CMakeLists.txt` and served from route handlers in `main/web_server.c`.
- `main/static/app.js` talks to device-local APIs with JSON over HTTP.
- Core pages are `/login`, `/dashboard`, `/status`, and `/wifi`.
- API routes are `/api/login`, `/api/logout`, `/api/status`, `/api/wifi/scan`, `/api/wifi/connect`, `/api/wifi/disconnect`, and authenticated pump routes under `/api/pump/*`.
- Pump API routes are GET/POST `/api/pump/config`, GET `/api/pump/status`, POST `/api/pump/start`, and POST `/api/pump/stop`.

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
- Stored pump keys live in namespace `pump_cfg`: Timer 1/2 ON/OFF seconds, relay polarity, and auto-start.
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
- `components/pump_control/` owns relay GPIO, binary float input, timer phase state, and start/stop behavior.
- Default pump hardware settings are GPIO32 for the active-low float input and GPIO26 for the relay output with configurable polarity.
- Manual hardware flashing and relay/float validation remain future validation work; Phase 3 only added authenticated API access and build validation.

## External Services Not Present

- No cloud service integration.
- No MQTT.
- No OTA update service.
- No remote database.
- No HTTPS certificate integration.
- No WebSocket.
- No filesystem such as SPIFFS or LittleFS.

## Incremental Refresh: Product Integration Contract

This section was refreshed from `.planning/` and `PRODUCT.md` only. Runtime
integration descriptions above remain from the prior full-map scan.

- `PRODUCT.md` confirms a local appliance boundary: owners and installers use
  the UI without internet access, so cloud services and remote assets remain
  excluded.
- `.planning/PROJECT.md` records the shipped v1.1 integration baseline:
  authenticated Thai-language dashboard and Hardware/Install page, SoftAP
  fallback, STA configuration, captive DNS, bounded diagnostics, dual pump
  relays, DS18B20 cooling, and active/pending hardware maps.
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-STATE-LANGUAGE.md`
  adds a shared presentation contract for loading, success, error, empty,
  disabled, unavailable, pending reboot, and disconnected states.
- `.planning/phases/11-baseline-ui-audit-and-state-language/11-PHASE-12-BRIEF.md`
  and `11-PHASE-13-BRIEF.md` preserve runtime behavior while improving local UI
  semantics and recovery guidance.

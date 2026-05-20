# Component Guide

## app_config

Header-only component with template-level constants. Downstream projects should
customize this first.

Important values:

- AP SSID and channel
- mDNS hostname and instance name
- default login credentials
- session timeout (`0` disables automatic expiry)
- AP auto-stop default
- HTTP route capacity
- watchdog timeout

## nvs_store

Persists Wi-Fi and optional static STA IP settings.

Namespace: `wifi_cfg`

Keys:

- `sta_ssid`
- `sta_pass`
- `sta_ip`
- `sta_gw`
- `sta_netmask`
- `sta_dns`
- `ap_stop_tmo`
- `ap_auto_stop`

## session

Small RAM-only session store for the local web UI.

- Token length: 16 hex chars plus null terminator
- Default max age: `APP_TEMPLATE_SESSION_MAX_AGE_SEC` (`0` means no automatic timeout)
- No reboot persistence
- Intended for local setup only

## wifi_manager

Owns Wi-Fi init and runtime state.

- Starts AP+STA mode
- Starts SoftAP immediately
- Loads saved STA credentials from NVS
- Scans Wi-Fi asynchronously
- Saves new STA credentials on connect
- Restores AP fallback after STA failures
- Optionally stops AP after STA gets IP

Public API is declared in `components/wifi_manager/wifi_manager.h`.

## main web server

`main/web_server.c` is intentionally project-level glue because it references
embedded static file symbols from `main/CMakeLists.txt`.

Template-owned routes:

- login/logout
- dashboard/status/wifi static pages
- Wi-Fi scan/connect/disconnect APIs
- status JSON API

Add product-specific pages and APIs here, but keep auth/session and Wi-Fi route
behavior stable unless the template contract changes.

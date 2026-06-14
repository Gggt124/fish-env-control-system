<!-- generated-by: gsd-doc-writer -->
# Configuration

## System Configuration

The firmware configuration is primarily managed through C preprocessor macros defined in `components/app_config/app_config.h` and ESP-IDF defaults in `sdkconfig.defaults`.

### Template Configuration (`app_config.h`)

This file contains constants for product names, Wi-Fi parameters, session limits, HTTP server configuration, and hardware GPIO mappings.

| Constant | Description | Default |
|----------|-------------|---------|
| `APP_TEMPLATE_NAME` | The display name of the project | `"Fish Pump Relay Timer Control"` |
| `APP_TEMPLATE_AP_SSID` | The SoftAP SSID for setup mode | `"FishPump-Setup"` |
| `APP_TEMPLATE_DEFAULT_USERNAME` | Default login username | `"admin"` |
| `APP_TEMPLATE_DEFAULT_PASSWORD` | Default login password | `"admin123"` |
| `APP_TEMPLATE_PUMP_FLOAT_GPIO` | GPIO pin for the binary float switch | `32` |
| `APP_TEMPLATE_PUMP_RELAY_GPIO` | GPIO pin for the pump relay | `26` |
| `APP_TEMPLATE_PUMP_TIMER1_ON_SEC` | Default Timer 1 ON phase duration | `20` |
| `APP_TEMPLATE_PUMP_TIMER1_OFF_SEC` | Default Timer 1 OFF phase duration | `60` |

### ESP-IDF Configuration (`sdkconfig.defaults`)

This file configures the underlying ESP-IDF framework options. It is used as a base during `idf.py build`.

| Variable | Description |
|----------|-------------|
| `CONFIG_IDF_TARGET` | The target MCU (`esp32`) |
| `CONFIG_PARTITION_TABLE_CUSTOM_FILENAME` | Points to `partitions.csv` for custom OTA layout |
| `CONFIG_ESPTOOLPY_FLASHSIZE_4MB` | Defines the board's flash size |
| `CONFIG_LWIP_MAX_SOCKETS` | Set to 16 to handle HTTP server and DNS needs |

## Required vs Optional Settings

- **Hardware GPIO Pins**: Required. Ensure that `APP_TEMPLATE_PUMP_FLOAT_GPIO` and `APP_TEMPLATE_PUMP_RELAY_GPIO` match your physical wiring. An incorrect relay pin can unexpectedly energize the pump.
- **Wi-Fi Credentials**: Optional at build time. The default `APP_TEMPLATE_AP_SSID` provides a SoftAP fallback for initial configuration.
- **Auto-Start**: Governed by `APP_TEMPLATE_PUMP_AUTO_START_DEFAULT` (default `false`). Required for safety reasons so the pump doesn't automatically start upon power loss recovery unless explicitly configured by the user.

## Per-Environment Overrides

There are no dynamic environment variable files (like `.env`) since this is an embedded C application. To change configurations for different deployment environments or boards:

1. Modify `app_config.h` directly for application-level limits and pins.
2. Use `idf.py menuconfig` to override ESP-IDF level features, which generates an `sdkconfig` file specific to your local build environment.

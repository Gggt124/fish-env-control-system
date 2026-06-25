<!-- generated-by: gsd-doc-writer -->
# Fish Pump Relay Timer Control

Firmware for an ESP32 DevKit V1 30-pin board that controls a fish pump relay using two configurable timers and one binary float switch.

## Installation

This project uses the ESP-IDF framework. You must have ESP-IDF installed and exported.

```bash
# Setup the target for ESP32
idf.py set-target esp32

# Build the project
idf.py build

# Flash and monitor (replace COMx with your serial port)
idf.py -p COMx flash monitor
```

> **Warning:** The partition table has been updated (the `nvs` partition was expanded). If you are flashing an existing device, you **must** run `idf.py -p COMx erase-flash` before flashing to prevent NVS and OTA state corruption.

## Quick Start

1. Connect your ESP32 DevKit V1 to your computer.
2. Build and flash the firmware using `idf.py build` and `idf.py -p COMx flash`.
3. Connect your device to the Wi-Fi AP named `FishPump-Setup` (IP: `192.168.4.1`).
4. Access the web dashboard at `http://192.168.4.1` with default credentials `admin` / `admin123`.

## Usage Examples

### Web Dashboard
The device runs a local web dashboard accessible via AP (`192.168.4.1`) or connected STA IP. You can configure the active Wi-Fi, view status, and interact with the controller.

### Hardware Control Logic
- **Float OFF**: Selects Timer 1.
- **Float ON**: Selects Timer 2.
- The relay follows the active timer's ON/OFF cycle to energize the pump accordingly.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

<!-- generated-by: gsd-doc-writer -->
# Fish Pump Relay Timer Control

Firmware for ESP32 (classic) and ESP32-S3 boards that controls a fish pump relay using two configurable timers and one binary float switch.

## Installation

This project uses the ESP-IDF framework and supports multiple hardware profiles. You must have ESP-IDF installed and exported.

We use PowerShell scripts to manage the dual-target builds.

```powershell
# Build for specific profile
.\scripts\build.ps1 -Target esp32
.\scripts\build.ps1 -Target esp32s3

# Create a release package containing binaries for all targets
.\scripts\package.ps1 -Target all -Zip

# Flash and monitor using an interactive menu
.\scripts\flash_and_show.ps1
```

> **Warning:** The partition table has been updated (the `nvs` partition was expanded). If you are flashing an existing device, you **must** run `idf.py -p COMx erase-flash` before flashing to prevent NVS and OTA state corruption.

## Quick Start

1. Connect your ESP32 or ESP32-S3 board to your computer.
2. Build the firmware using `.\scripts\build.ps1 -Target <profile>` and flash it using `.\scripts\flash_and_show.ps1`.
3. Connect your device to the Wi-Fi AP named `FishPump-Setup` (IP: `192.168.4.1`).
4. Access the web dashboard at `http://192.168.4.1` with default credentials `admin` / `admin123`.

## Building for ESP32-S3

This project includes a dedicated hardware profile for the **ESP32-S3-DevKitC-1 WROOM-1-N16R8** (16 MB Flash, 8 MB Octal PSRAM).

- **Build command**: `.\scripts\build.ps1 -Target esp32s3`
- **Output directory**: `build-esp32s3\`
- **Flash-package**: run `FLASH.bat` and select option `2` for ESP32-S3

> **Warning:** **S3 USB Console**: The S3 profile routes monitor output to the native USB Serial/JTAG port. **Connect your cable to the "USB" port** on the DevKitC-1, not the "UART" port. If you use the UART port, you will see no monitor output!

When monitoring the boot log, you should see PSRAM detection:
```text
esp_psram: Found 8MB PSRAM device
```

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

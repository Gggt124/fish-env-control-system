<!-- generated-by: gsd-doc-writer -->
# Getting Started

## Prerequisites

- **ESP-IDF v6.0.1** (or compatible ESP-IDF version). This project is not compatible with Arduino or PlatformIO.
- **Python** (required for ESP-IDF tools).
- An ESP32 DevKit V1 30-pin board (or equivalent classic ESP32).

## Installation Steps

1. Clone the repository:
   ```bash
   git clone <repository_url>
   cd fish-pump-RelayTimerControl
   ```

2. Set up the ESP-IDF environment. On Windows:
   ```powershell
   $env:IDF_PATH = "C:\esp\v6.0.1\esp-idf"  # Update path as necessary
   & "$env:IDF_PATH\export.ps1"
   ```

3. Ensure correct charset in PowerShell to avoid build warnings:
   ```powershell
   chcp 65001 > $null
   ```

4. Set the target to esp32 (one-time setup):
   ```bash
   idf.py set-target esp32
   ```

## First Run

Build and flash the firmware:

```bash
idf.py build
idf.py -p COMx flash monitor  # Replace COMx with your serial port
```

Alternatively, you can use the provided build script on Windows:
```powershell
.\scripts\build.ps1
```

Once flashed, the device will start a SoftAP named `FishPump-Setup`. Connect to it and navigate to `http://192.168.4.1` to access the setup UI.

## Common Setup Issues

- **Unicode warnings during build**: ESP-IDF requires UTF-8. Run `chcp 65001` in your PowerShell session before building.
- **Python dependencies missing**: Run `python -m pip install -r "$env:IDF_PATH\tools\requirements\requirements.core.txt"`.
- **Flash failures**: If the board was previously flashed with flash encryption enabled, you may need to disable it or burn `FLASH_CRYPT_CNT` so the value becomes even.

## Next Steps

- Check out [DEVELOPMENT.md](DEVELOPMENT.md) for local development instructions.
- See [ARCHITECTURE.md](ARCHITECTURE.md) to understand the system structure.

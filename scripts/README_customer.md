# Firmware Installation Guide (Fish Pump)

This folder contains the firmware and updater tools for your Fish Pump control board.

## Prerequisites
1. **Windows 10 or Windows 11** PC.
2. A USB cable to connect the ESP32 board.
3. **Python 3** installed (Download from https://www.python.org/downloads/ if you don't have it).

## How to Flash (Update)
1. Plug the USB cable into your computer and the ESP32 board.
2. Double-click the **`FLASH.bat`** file.
3. A black command window will open.
   - If prompted for a **COM port**, type the corresponding port (e.g., `COM5`) and press Enter.
   - The script will automatically install necessary tools (esptool) and flash the firmware.
4. Wait for the success message: **"[OK] Flash สำเร็จ!"** (Flash Successful).
5. The script will display your unique **Wi-Fi Password**. Please save it!

## Forgot Wi-Fi Password?
Your Wi-Fi AP password is automatically generated based on your board's hardware MAC address. 
If you forget it:
1. Connect the board via USB.
2. Double-click **`README_password.bat`**.
3. The script will read the hardware ID and display your Wi-Fi password without reflashing.

## Default Credentials
- **Wi-Fi SSID:** `FishPump-Setup`
- **Dashboard URL:** `http://192.168.4.1`
- **Admin Login:**
  - Username: `admin`
  - Password: `admin123`

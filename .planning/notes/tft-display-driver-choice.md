---
title: "TFT Display Driver Choice and Pin Mapping"
date: "2026-06-06"
context: "Exploration of adding a 2.4\" TFT display (ILI9341) for real-time status on a standalone vertical case"
---

# TFT Display Driver Choice and Pin Mapping

We have decided to integrate a 2.4" TFT SPI 240x320 display (ILI9341 controller) to serve as a read-only real-time status monitor. 

## Key Technical Decisions

### 1. Driver and UI Library
We will use the native **ESP-IDF `esp_lcd`** component (with the `esp_lcd_ili9341` driver) instead of integrating LVGL. 
* **RAM Protection**: Classic ESP32 has limited SRAM. Native `esp_lcd` uses <2KB of DRAM, whereas LVGL v9.x requires 30-60KB. Keeping the RAM footprint low preserves stability for the web server, Wi-Fi manager, and captive DNS.
* **Stateless Drawing**: A simple, custom-written drawing wrapper will update only changed text and numbers, preventing heap fragmentation and rendering lag.

### 2. GPIO Pin Mapping (ESP32 DevKit V1 30-Pin)
To avoid conflict with existing sensors and relays, we will map the SPI and control lines to the standard VSPI bus and unused general-purpose outputs:

| Display Pin | Function | ESP32 GPIO | Status / Description |
|-------------|----------|------------|----------------------|
| **VCC**     | Power    | 3V3 / 5V   | Connect to 3.3V or 5V power rail |
| **GND**     | Ground   | GND        | Connect to common ground |
| **CS**      | TFT CS   | **GPIO5**  | VSPI Chip Select (strapping pin, safe) |
| **RESET**   | TFT Reset| **GPIO22** | General purpose output |
| **DC**      | TFT D/C  | **GPIO21** | General purpose output |
| **SDI(MOSI)**| TFT MOSI| **GPIO23** | VSPI MOSI (hardware SPI) |
| **SCK**     | TFT Clock| **GPIO18** | VSPI SCK (hardware SPI) |
| **LED**     | Backlight| **GPIO4**  | Backlight control (or tie to 3.3V for always-on) |
| **SDO(MISO)**| TFT MISO| **GPIO19** | VSPI MISO (not strictly needed for read-only, can leave disconnected) |

*Touch screen pins (`T_CS`, `T_CLK`, `T_DIN`, `T_DO`, `T_IRQ`) and SD card pins (`SD_CS`, `SD_MOSI`, `SD_MISO`, `SD_SCK`) are left disconnected for this phase to preserve GPIOs and simplify code.*

### Existing Pin Reference (Unchanged)
* **GPIO32**: Float switch input (internal pull-up, active-low)
* **GPIO26**: Pump Relay 1
* **GPIO27**: Pump Relay 2
* **GPIO33**: DS18B20 temperature sensor data
* **GPIO25**: Cooling Relay

---

## Screen Layout Design (Landscape - 320x240)

The screen will be partitioned into a dual-column layout to organize status indicators:

```
+-------------------------------------------------------------+
| [WiFi: AP/STA]  IP: 192.168.X.X             [Uptime: H:M:S] |
+------------------------------+------------------------------+
|         PUMP CHANNEL         |        COOLING CHANNEL       |
|  +------------------------+  |  +------------------------+  |
|  |     STATUS: RUNNING    |  |  |       28.5 °C          |  |
|  +------------------------+  |  +------------------------+  |
|  Active: Timer 1 (Float LOW) |  Cooling Relay: ON           |
|  Phase: ON                   |  Mode: Auto                  |
|  Remaining: 00:45            |  Compressor Lockout: OFF     |
+------------------------------+------------------------------+
```

### Visual and Color Codes
* **Active/On Status** (e.g., Pump Running, Cooling ON): Highlighted in **Green** or high-contrast White.
* **Inactive/Off Status** (e.g., Pump Stopped, Cooling OFF): Rendered in dim **Gray**.
* **Alerts/Warnings** (e.g., Sensor Fault, Disconnected): Displayed in **Red** or **Yellow**.

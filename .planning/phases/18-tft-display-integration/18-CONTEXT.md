# Phase 18: TFT Display Integration - Context

**Gathered:** 2026-06-06
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase delivers the integration of a 2.4" TFT (ILI9341) screen in landscape orientation (320x240) to serve as a read-only real-time status display. It includes SPI VSPI bus and `esp_lcd` initialization, a lightweight text and shape drawing wrapper, a dedicated FreeRTOS update task, and a status dashboard screen.

</domain>

<decisions>
## Implementation Decisions

### Layout & Screen Refresh Rate
- **D-01:** Implement a hybrid refresh rate. The countdown timer ticks down smoothly every 1 second, while other binary states (pump ON/OFF, float switch state, temperature, Wi-Fi IP) are updated immediately upon transition/change to maximize user responsiveness.
- **D-02:** Use a dual-column landscape layout (320x240): Left column displays Pump state and active timer countdowns; Right column displays temperature, cooling relay state, float switch status, and Wi-Fi IP.

### Backlight Power Management
- **D-03:** The screen backlight (LED pin GPIO4) will default to always-on during device operation to act as a physical panel dashboard.

### Display UI Language
- **D-04:** The TFT UI will use English labels and numbers (e.g. `PUMP: ON`, `TEMP: 28.5 C`, `COUNTDOWN: 00:45`, `IP: 192.168.4.1`) to avoid the RAM and processing overhead of loading custom Thai fonts on a raw LCD canvas.

### the agent's Discretion
- The executor has discretion over the visual design details (borders, font sizes, icons, color highlights) as long as they follow the HSL color palette principles and are highly readable from a distance.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Display Configuration & Pinout
- `.planning/notes/tft-display-driver-choice.md` — TFT display driver selection (native `esp_lcd`), pinout mapping, and wireframe layout.

### Product UI/UX and Brand Guidelines
- `PRODUCT.md` §Anti-references — enforces clean, functional layouts, and prohibits decorative animations or excessive visual weight.
- `PRODUCT.md` §Design Principles — guidelines on presenting immediate status decisions and keeping states explicit.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `main/web_server.c` §`handle_api_status` — compiles the system status JSON. We can tap into the same underlying global status structures (e.g., wifi_manager states, pump controller state) to obtain clean values for the display.
- HSL Color Palette (from `main/static/style.css`) — use corresponding colors (Green for active, Gray for inactive, Red/Yellow for warnings) in the C display code.

### Established Patterns
- Component boundaries — Keep the display driver in a separate main-level C module (e.g., `main/tft_display.c` / `main/tft_display.h`) that registers a FreeRTOS task, keeping `app_main.c` clean.

### Integration Points
- `main/app_main.c` §`app_main` — the display task should be spawned after the NVS and session store are initialized, but before Wi-Fi or Web Server initialization, to show boot status on screen.

</code_context>

<specifics>
## Specific Ideas

- The display should show a brief "Booting..." screen with the project name and version immediately upon power-on before transitions to the main status screen.

</specifics>

<deferred>
## Deferred Ideas

- Touch screen interaction — deferred to a future UI phase (Touch CS pin GPIO13 is reserved but not wired).
- SD card logging — deferred to a future storage phase (SD CS pin GPIO12 is reserved but not wired).

</deferred>

---

*Phase: 18-TFT Display Integration*
*Context gathered: 2026-06-06*

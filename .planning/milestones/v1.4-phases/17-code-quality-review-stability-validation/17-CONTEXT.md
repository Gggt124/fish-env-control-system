# Phase 17: Code Quality Review & Stability Validation - Context

**Gathered:** 2026-06-06
**Status:** Ready for planning

<domain>
## Phase Boundary

Review firmware source and web assets to resolve warnings, console errors, and ensure runtime stability. The firmware must compile cleanly under ESP-IDF, the web UI must run without errors in the browser, and the controller must operate stably during manual regression checks of the pump and cooling systems.

</domain>

<decisions>
## Implementation Decisions

### Compiler Warnings Treatment
- **D-01:** Address all warnings in `main/` and custom components (`components/`). Do not touch warnings in managed components.
- **D-02:** Use default ESP-IDF warning configurations; do not add extra compiler warning flags.

### JavaScript Errors & Console Logs
- **D-03:** Clean up temporary/debug `console.log` statements in `app.js`. Keep necessary diagnostic logs for key operations.
- **D-04:** Eliminate all JavaScript console errors and warnings in the browser.

### Firmware Logging & Diagnostics
- **D-05:** Maintain `ESP_LOG_INFO` as the default log level for standard operations, logging critical state transitions clearly.
- **D-06:** Keep existing structured diagnostics in status JSON/UI, ensuring robust uptime and bounds calculations.

### Task Watchdog & Recovery
- **D-07:** Verify the main task and critical control tasks feed/reset the watchdog correctly to prevent CPU starvation. Keep default 10s watchdog timeout.
- **D-08:** Rely on standard ESP-IDF boot-time reset reason logging; do not add extra NVS crash storage.

### the agent's Discretion
- Spacing and style of console logs.
- Refactoring details to resolve specific warnings, provided they do not break existing functionality or soak test baseline stability.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- The project has modular components (`wifi_manager`, `nvs_store`, `session`) and main files (`web_server.c`, `app_main.c`) with clear interfaces.

### Established Patterns
- Strict separation of concern between the low-level modules and high-level routing.
- Uptime calculations use standard 64-bit integer values in milliseconds.
- Uptime diagnostic bounds check prevents negative/incorrect values in UI.

### Integration Points
- `app_main.c` main task watchdog reset loop (runs every 5 seconds).
- `web_server.c` diagnostic API endpoint `/api/status`.
- `app.js` UI polling of `/api/status`.

</code_context>

<specifics>
## Specific Ideas

- Ensure clean rebuild without warnings using `powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -FullClean`.
- Test static files and JS in browser to verify zero console errors.

</specifics>

<deferred>
## Deferred Ideas

- None — discussion stayed within phase scope.

</deferred>

# Cross-AI Plan Review Request

You are reviewing implementation plans for a software project phase.
Provide structured feedback on plan quality, completeness, and risks.

## Project Context
# Fish Pump Relay Timer Control

## What This Is

Firmware for an ESP32 DevKit V1 30-pin board that controls two fish-pump relay channels from one binary float switch and a separate DS18B20 cooling relay. The local web dashboard is a professional, accessible offline control surface for timer settings, pump operation, cooling settings, guarded hardware mapping, Wi-Fi setup, and runtime diagnostics â€” with clear operator hierarchy, responsive mobile layout, and consistent state communication.

The control logic is intentionally simple and hardware-real: the float switch is an ON/OFF contact, not a continuous water-level sensor. Float ON selects Timer 1/Relay 1, Float OFF selects Timer 2/Relay 2, and only the selected pump relay follows its timer's ON/OFF phase.

## Core Value

The pump must switch reliably between Timer 1 and Timer 2 based on the float switch and drive the relay safely according to the selected timer's ON/OFF cycle.

## Milestone Status

**Latest shipped:** v1.5 TFT Display Integration on 2026-06-06

## Current Milestone: v1.7 Authentication & Recovery

**Goal:** Improve access control by adding persistent sessions, user credential management, and a recovery mechanism for forgotten credentials.

**Target features:**
- AUTH-01: "Remember Me" persistent session to bypass login on trusted devices.
- AUTH-02: User interface to change Username & Password.
- AUTH-03: Credential Recovery mechanism (e.g., hardware button reset or fallback) to prevent lockout.

## Requirements

### Validated

- âœ“ Device boots as ESP-IDF firmware on classic ESP32 target â€” existing
- âœ“ Local web server serves embedded HTML/CSS/JS without external dependencies â€” existing
- âœ“ Login/session flow protects dashboard, status, Wi-Fi, and API routes â€” existing
- âœ“ SoftAP fallback and Wi-Fi STA setup are available through the local UI â€” existing
- âœ“ Wi-Fi credentials and optional static IP configuration persist in NVS â€” existing
- âœ“ Device status JSON reports system, Wi-Fi, heap, uptime, and service state â€” existing
- âœ“ Pump control core initializes GPIO26 relay output inactive during boot and leaves control stopped â€” v1.0
- âœ“ Pump control core defines GPIO32 active-low binary float input with debounce for timer selection â€” v1.0
- âœ“ Pump control core implements Timer 1/Timer 2 ON/OFF defaults, bounds, and relay phase behavior â€” v1.0
- âœ“ ESP-IDF build succeeds with pump control integrated â€” v1.0
- âœ“ Timer 1/Timer 2 ON/OFF settings persist in NVS through the `pump_cfg` namespace â€” v1.0
- âœ“ Pump boot behavior loads persisted settings before pump initialization and defaults safely when missing â€” v1.0
- âœ“ Auto-start is persisted at the firmware layer and disabled saved settings suppress boot start â€” v1.0
- âœ“ Authenticated `/api/pump/config` returns and saves validated pump settings while keeping GPIO/debounce read-only â€” v1.0
- âœ“ Authenticated `/api/pump/status` returns machine-friendly pump runtime state â€” v1.0
- âœ“ Authenticated `/api/pump/start` and `/api/pump/stop` provide idempotent runtime controls with same-origin POST protection â€” v1.0
- âœ“ User can configure Timer 1 and Timer 2 ON/OFF durations from the web UI â€” v1.0
- âœ“ User can start and stop pump control from the web UI â€” v1.0
- âœ“ User can choose whether pump control auto-starts on boot from authenticated API and web UI controls â€” v1.0
- âœ“ Dashboard displays active timer, current phase, countdown, float state, relay state, and auto-start state â€” v1.0
- âœ“ Firmware preserves AP/Wi-Fi setup access while pump control is running â€” v1.0
- âœ“ Manual hardware test confirms float OFF selects Timer 1 and float ON selects Timer 2 â€” v1.0
- âœ“ Manual hardware test confirms relay follows ON/OFF phase and Stop forces inactive â€” v1.0
- âœ“ Manual reboot test confirms timer settings and auto-start preference persist â€” v1.0
- âœ“ Manual access test confirms Wi-Fi setup, SoftAP fallback, login, and status pages still work while pump control is present â€” v1.0
- âœ“ Firmware-defined safe GPIO options exist for float input, pump Relay 1, pump Relay 2, DS18B20 data, and cooling relay â€” v1.1
- âœ“ Conservative ESP32 DevKit V1 defaults are GPIO32, GPIO26, GPIO27, GPIO33, and GPIO25 for the hardware roles â€” v1.1
- âœ“ Relay 1, Relay 2, and cooling relay polarity are represented independently in persistent settings â€” v1.1
- âœ“ DS18B20 powered-mode wiring assumptions are documented with 3.3 V rail and 4.7 kOhm pull-up â€” v1.1
- âœ“ Active and pending GPIO maps, relay polarity, cooling settings, and timer start phases are stored through documented NVS schemas â€” v1.1
- âœ“ Pump control uses two separate timer/relay channels selected by binary float state â€” v1.1
- âœ“ Float state changes restart the selected timer cycle and force the previously selected relay OFF â€” v1.1
- âœ“ Temperature cooling control uses DS18B20 and a dedicated relay separate from pump relays â€” v1.1
- âœ“ Cooling control provides explicit enable, auto-enable preference, safe override modes, and compressor protection â€” v1.1
- âœ“ Web Hardware/Install GPIO map exposes firmware-defined safe enum options and pending reboot behavior â€” v1.1
- âœ“ Baseline UI audit, Thai-first shared state language, offline ESP32 checklist, and scoped implementation briefs are established before frontend edits â€” v1.2
- âœ“ Owner dashboard clearly presents pump state, selected timer/relay, countdown, float switch, and cooling as separate operational channels â€” v1.2
- âœ“ Consistent app shell navigation with mobile drawer across all authenticated pages â€” v1.2
- âœ“ Login presents clear credential entry, loading, error, and recovery guidance â€” v1.2
- âœ“ Hardware/Install distinguishes active, pending, and reboot-required GPIO state with DS18B20 pull-up guidance â€” v1.2
- âœ“ Wi-Fi scan/connect flow presents loading, error, empty, disconnected, and connected states â€” v1.2
- âœ“ Keyboard focus visibility and assistive-technology status announcements across all pages â€” v1.2
- âœ“ Professional CSS overhaul with HSL color system, accessible contrast, no CDN or external assets â€” v1.2
- âœ“ Full visual regression validated with screenshots, accessibility audit, build/footprint gate, and device-backed hardware regression â€” v1.2
- âœ“ UI-17: Only list changed GPIO pins in the Pending Reboot Map display â€” v1.3
- âœ“ UI-18: GPIO map unsaved warning disappears and button disables on value reversion â€” v1.3
- âœ“ UI-19: Wi-Fi disconnect button simplified to "Disconnect" / "Disconnecting..." â€” v1.3
- âœ“ UI-20: Wi-Fi disconnect requires confirmation dialog â€” v1.3
- âœ“ UI-21: Pump and Cooling Settings save buttons disabled by default â€” v1.3
- âœ“ UI-22: Pump and Cooling Settings forms unsaved warning and button disable on value reversion â€” v1.3
- âœ“ UI-23: Logout menu requires confirmation dialog â€” v1.3

## Phase 3: Hardware Recovery & Anti-Lockout
### Roadmap Section
### Phase 3: Hardware Recovery & Anti-Lockout

**Goal**: Provide physical hardware fail-safes for lost credentials and connectivity loss.
**Depends on**: Phase 2
**Requirements**: RECOV-01, RECOV-02, RECOV-03
**Success Criteria** (what must be TRUE):

  1. User can hold the recovery button to reset credentials to dmin/dmin123 without losing other settings.
  2. SoftAP fallback only opens when user physically presses the recovery button.
  3. If new credentials or Wi-Fi settings fail validation, the device reboots and rolls back to previous known-good settings.

**Plans**: 4 plans

- [ ] 03-01-PLAN.md — SoftAP manual trigger and timeout
- [ ] 03-02-PLAN.md — NVS staging and factory reset
- [ ] 03-03-PLAN.md — API integration and confirm endpoint
- [ ] 03-04-PLAN.md — Hardware button UI and boot coordination

### Requirements Addressed
- [ ] **RECOV-01:** Hardware Factory Reset. Pressing a designated GPIO button (active-low, 10k pull-up, debounced) for 5-10 seconds must reset credentials to `admin`/`admin123` without affecting Wi-Fi or pump configurations.
- [ ] **RECOV-02:** Timed Setup with Physical Consent (Secure AP Fallback). SoftAP (`FishPump-Setup`) must NOT open automatically on STA failure. It only opens for 5 minutes after the user physically presses the recovery button for 2 seconds.
- [ ] **RECOV-03:** Anti-Lockout Validation Rollback. Credential/Wi-Fi updates write to a temporary NVS staging namespace. The ESP32 reboots and waits 3 minutes for a user "Confirm" API call. If unconfirmed, it rolls back to the previous known-good credentials.
| RECOV-01 | Phase 3 | Pending |
| RECOV-02 | Phase 3 | Pending |
| RECOV-03 | Phase 3 | Pending |

### User Decisions (CONTEXT.md)
# Phase 3: Hardware Recovery & Anti-Lockout - Context

**Gathered:** 2026-06-13
**Status:** Ready for planning

<domain>
## Phase Boundary

Provide physical hardware fail-safes for lost credentials and connectivity loss. This phase implements the hardware inputs and system recovery flows (credential reset, SoftAP fallback, connection validation rollback).

</domain>

<decisions>
## Implementation Decisions

### Recovery Button & Actions (à¸›à¸¸à¹ˆà¸¡ Recovery à¹à¸¥à¸°à¸ˆà¸±à¸‡à¸«à¸§à¸°à¸à¸²à¸£à¸à¸”)
- **D-01:** à¸£à¸­à¸‡à¸£à¸±à¸šà¸›à¸¸à¹ˆà¸¡ 2 à¸Šà¸¸à¸”: à¸›à¸¸à¹ˆà¸¡ Boot à¸ à¸²à¸¢à¹ƒà¸™ (GPIO 0) à¹à¸¥à¸°à¸›à¸¸à¹ˆà¸¡à¸ à¸²à¸¢à¸™à¸­à¸à¸—à¸µà¹ˆà¸à¸³à¸«à¸™à¸”à¸‚à¸²à¹„à¸”à¹‰ (à¹€à¸›à¸´à¸”à¹ƒà¸Šà¹‰à¸‡à¸²à¸™ internal pull-up)
- **D-02:** à¸›à¹‰à¸­à¸‡à¸à¸±à¸™à¸à¸²à¸£à¸à¸”à¸žà¸£à¹‰à¸­à¸¡à¸à¸±à¸™ (Mutual Exclusion): à¸«à¸²à¸à¸›à¸¸à¹ˆà¸¡à¸«à¸™à¸¶à¹ˆà¸‡à¸–à¸¹à¸à¸à¸”à¸­à¸¢à¸¹à¹ˆ à¹ƒà¸«à¹‰à¸¥à¸°à¹€à¸§à¹‰à¸™à¸à¸²à¸£à¸­à¹ˆà¸²à¸™à¸„à¹ˆà¸²à¸ˆà¸²à¸à¸­à¸µà¸à¸›à¸¸à¹ˆà¸¡à¹€à¸žà¸·à¹ˆà¸­à¸›à¹‰à¸­à¸‡à¸à¸±à¸™à¸£à¸°à¸šà¸šà¸£à¸§à¸™
- **D-03:** à¸à¸”à¸„à¹‰à¸²à¸‡ 2 à¸§à¸´à¸™à¸²à¸—à¸µ: à¹€à¸›à¸´à¸”à¹‚à¸«à¸¡à¸” SoftAP à¸­à¸¢à¹ˆà¸²à¸‡à¹€à¸”à¸µà¸¢à¸§ (à¸ªà¸³à¸«à¸£à¸±à¸šà¹ƒà¸Šà¹‰à¹€à¸›à¸¥à¸µà¹ˆà¸¢à¸™ Wi-Fi à¹‚à¸”à¸¢à¹„à¸¡à¹ˆà¸£à¸µà¹€à¸‹à¹‡à¸•à¸£à¸«à¸±à¸ªà¸œà¹ˆà¸²à¸™)
- **D-04:** à¸à¸”à¸„à¹‰à¸²à¸‡ 5 à¸§à¸´à¸™à¸²à¸—à¸µ: à¹€à¸›à¸´à¸”à¹‚à¸«à¸¡à¸” SoftAP à¹à¸¥à¸°à¸—à¸³à¸à¸²à¸£à¸£à¸µà¹€à¸‹à¹‡à¸•à¸£à¸«à¸±à¸ªà¸œà¹ˆà¸²à¸™à¸à¸¥à¸±à¸šà¹€à¸›à¹‡à¸™ `admin`/`admin123` (Recovery)

### LED Feedback (à¸à¸²à¸£à¹à¸ªà¸”à¸‡à¸œà¸¥à¹„à¸Ÿ LED)
- **D-05:** à¸£à¸­à¸‡à¸£à¸±à¸šà¹„à¸Ÿ LED 2 à¸Šà¸¸à¸”: à¹„à¸Ÿà¸ªà¸µà¸Ÿà¹‰à¸²à¸šà¸™à¸šà¸­à¸£à¹Œà¸” à¹à¸¥à¸°à¹„à¸Ÿ LED à¸ à¸²à¸¢à¸™à¸­à¸à¸—à¸µà¹ˆà¸à¸³à¸«à¸™à¸”à¸‚à¸²à¹„à¸”à¹‰
- **D-06:** à¹à¸ªà¸”à¸‡à¸ªà¸–à¸²à¸™à¸°à¸à¸²à¸£à¸à¸”à¸”à¸±à¸‡à¸™à¸µà¹‰:
  - 0-2 à¸§à¸´à¸™à¸²à¸—à¸µ: à¹„à¸Ÿà¸•à¸´à¸”à¸„à¹‰à¸²à¸‡ (Solid ON)
  - 2-5 à¸§à¸´à¸™à¸²à¸—à¸µ: à¹„à¸Ÿà¸à¸°à¸žà¸£à¸´à¸šà¸Šà¹‰à¸² (Slow Blink) à¹à¸ªà¸”à¸‡à¸§à¹ˆà¸²à¸–à¸¶à¸‡à¹€à¸à¸“à¸‘à¹Œà¹€à¸›à¸´à¸” AP
  - > 5 à¸§à¸´à¸™à¸²à¸—à¸µ: à¹„à¸Ÿà¸à¸°à¸žà¸£à¸´à¸šà¹€à¸£à¹‡à¸§ (Fast Blink) à¹à¸ªà¸”à¸‡à¸§à¹ˆà¸²à¸–à¸¶à¸‡à¹€à¸à¸“à¸‘à¹Œ Recovery
- **D-07:** à¹€à¸¡à¸·à¹ˆà¸­à¸›à¸¥à¹ˆà¸­à¸¢à¸›à¸¸à¹ˆà¸¡à¹à¸¥à¸° SoftAP à¸à¸³à¸¥à¸±à¸‡à¹€à¸›à¸´à¸”à¹ƒà¸Šà¹‰à¸‡à¸²à¸™ à¹„à¸Ÿà¸ˆà¸°à¸•à¸´à¸”à¸„à¹‰à¸²à¸‡à¹€à¸žà¸·à¹ˆà¸­à¸šà¸­à¸à¸§à¹ˆà¸²à¸£à¸°à¸šà¸šà¸žà¸£à¹‰à¸­à¸¡à¹ƒà¸«à¹‰à¹€à¸Šà¸·à¹ˆà¸­à¸¡à¸•à¹ˆà¸­ à¹à¸¥à¸°à¸ˆà¸°à¸”à¸±à¸šà¹€à¸¡à¸·à¹ˆà¸­ SoftAP à¸›à¸´à¸”à¸•à¸±à¸§à¸¥à¸‡

### SoftAP Fallback Behavior (à¸žà¸¤à¸•à¸´à¸à¸£à¸£à¸¡à¸‚à¸­à¸‡ SoftAP)
- **D-08:** SoftAP à¸ˆà¸°à¸›à¸´à¸”à¸•à¸±à¸§à¹€à¸­à¸‡à¸­à¸±à¸•à¹‚à¸™à¸¡à¸±à¸•à¸´ (Timeout) à¸ à¸²à¸¢à¹ƒà¸™ 10 à¸™à¸²à¸—à¸µ à¹€à¸žà¸·à¹ˆà¸­à¹€à¸«à¸•à¸¸à¸œà¸¥à¸”à¹‰à¸²à¸™à¸„à¸§à¸²à¸¡à¸›à¸¥à¸­à¸”à¸ à¸±à¸¢
- **D-09:** Timeout à¸ˆà¸°à¸«à¸¢à¸¸à¸”à¸™à¸±à¸š (à¸«à¸£à¸·à¸­à¸–à¸¹à¸à¸¢à¸·à¸”à¸­à¸­à¸à¹„à¸›) à¸•à¸£à¸²à¸šà¹ƒà¸”à¸—à¸µà¹ˆà¸¢à¸±à¸‡à¸¡à¸µà¸­à¸¸à¸›à¸à¸£à¸“à¹Œà¹€à¸Šà¸·à¹ˆà¸­à¸¡à¸•à¹ˆà¸­à¸à¸±à¸š Wi-Fi 'FishPump-Setup' à¸„à¹‰à¸²à¸‡à¹„à¸§à¹‰

### Rollback Mechanism (à¸à¸¥à¹„à¸à¸à¸²à¸£à¸¢à¹‰à¸­à¸™à¸à¸¥à¸±à¸šà¸à¸²à¸£à¸•à¸±à¹‰à¸‡à¸„à¹ˆà¸² Wi-Fi)
- **D-10:** à¸«à¸²à¸à¸¡à¸µà¸à¸²à¸£à¸•à¸±à¹‰à¸‡à¸„à¹ˆà¸² Wi-Fi à¹ƒà¸«à¸¡à¹ˆ à¸£à¸°à¸šà¸šà¸ˆà¸°à¸£à¸­à¸£à¸±à¸š IP Address à¹€à¸›à¹‡à¸™à¹€à¸§à¸¥à¸² 30 à¸§à¸´à¸™à¸²à¸—à¸µ
- **D-11:** à¸«à¸²à¸à¹„à¸¡à¹ˆà¹„à¸”à¹‰à¸£à¸±à¸š IP à¸ à¸²à¸¢à¹ƒà¸™ 30 à¸§à¸´à¸™à¸²à¸—à¸µ à¸ˆà¸°à¸–à¸·à¸­à¸§à¹ˆà¸²à¸¥à¹‰à¸¡à¹€à¸«à¸¥à¸§ à¸£à¸°à¸šà¸šà¸ˆà¸°à¸—à¸³à¸à¸²à¸£à¸£à¸µà¸šà¸¹à¸•à¹à¸¥à¸°à¸¢à¹‰à¸­à¸™à¸à¸¥à¸±à¸šà¹„à¸›à¹ƒà¸Šà¹‰à¸à¸²à¸£à¸•à¸±à¹‰à¸‡à¸„à¹ˆà¸² Wi-Fi à¸¥à¹ˆà¸²à¸ªà¸¸à¸”à¸—à¸µà¹ˆà¹ƒà¸Šà¹‰à¸‡à¸²à¸™à¹„à¸”à¹‰à¸—à¸±à¸™à¸—à¸µ

### the agent's Discretion
None

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Architecture & Conventions
- `.planning/PROJECT.md` â€” Project definition and stack context
- `.planning/REQUIREMENTS.md` â€” Requirement definitions (RECOV-01, RECOV-02, RECOV-03)
- `.planning/ROADMAP.md` â€” Milestone context

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `components/nvs_store` (nvs_store.h/c): Required to save or erase current configurations for rollback or reset features.
- `components/wifi_manager` (wifi_manager.h/c): Required to control APSTA mode, STA connection testing, and SoftAP timeout timers.
- `components/app_config` (app_config.h): Holds default admin credentials and timeouts constants. Needs new constants for external GPIOs.

### Established Patterns
- Wi-Fi logic utilizes FreeRTOS timers and ESP-IDF event loops (WIFI_EVENT/IP_EVENT).
- NVS keys (`sta_ssid`, `sta_pass`) are abstracted inside `nvs_store`.

### Integration Points
- `main/app_main.c`: Must integrate the GPIO button initialization, LED initialization, and FreeRTOS task/timer for polling buttons and controlling LEDs.
- `components/wifi_manager`: Must implement the 30-second STA IP wait state and the 10-minute SoftAP timeout state counting active AP clients.

</code_context>

<specifics>
## Specific Ideas

No specific requirements â€” open to standard approaches.

</specifics>

<deferred>
## Deferred Ideas

- **First Setup Guide**: Provide a wizard UI for the first-time setup (fresh flash) to force the user to change the default username and password. Deferred because it is a new UX capability outside the hardware recovery scope.

</deferred>

---

*Phase: 03-Hardware Recovery & Anti-Lockout*
*Context gathered: 2026-06-13*

### Plans to Review
## 03-01-PLAN.md
---
wave: 1
depends_on: []
files_modified:
  - components/app_config/app_config.h
  - components/wifi_manager/wifi_manager.h
  - components/wifi_manager/wifi_manager.c
autonomous: true
requirements:
  - RECOV-02: Timed Setup with Physical Consent
must_haves:
  truths:
    - SoftAP MUST stop automatically if no clients are connected after 10 minutes.
    - Timer MUST reset when new clients connect so active users are not dropped.
    - Manual trigger MUST be idempotent (can be called repeatedly without crashing).
  artifacts:
    - New constants in app_config.h
    - New function wifi_manager_start_recovery_ap
  key_links: []
---

# Phase 3: Hardware Recovery & Anti-Lockout

## Goal
Implement the SoftAP manual trigger and timeout mechanisms required for Secure AP Fallback. SoftAP should open for a limited time and reset its timeout when clients connect.

## Verification
- `idf.py build` succeeds.
- When `wifi_manager_start_recovery_ap()` is called, AP becomes visible.
- After 10 idle minutes, AP is hidden.

## Artifacts this phase produces
- New constants in `app_config.h`: `APP_CONFIG_AP_RECOVERY_TIMEOUT_MS`, `APP_CONFIG_AP_IDLE_TIMEOUT_MS`, `APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS`, `APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS`
- New function in `wifi_manager.h`: `wifi_manager_start_recovery_ap(void)`
- New state fields in `wifi_manager.c`: `s_ap_timeout_timer`, `s_ap_timeout_deadline_ms`

## Tasks

<task>
<read_first>
- components/app_config/app_config.h
</read_first>
<action>
Update `components/app_config/app_config.h` to add new timing constants for recovery and rollback features:
- `APP_CONFIG_AP_RECOVERY_TIMEOUT_MS` set to `300000` (5 minutes)
- `APP_CONFIG_AP_IDLE_TIMEOUT_MS` set to `600000` (10 minutes)
- `APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS` set to `30000` (30 seconds)
- `APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS` set to `180000` (3 minutes)
</action>
<acceptance_criteria>
- `components/app_config/app_config.h` contains `APP_CONFIG_AP_RECOVERY_TIMEOUT_MS` defined as 300000
</acceptance_criteria>
</task>

<task>
<read_first>
- components/wifi_manager/wifi_manager.h
- components/wifi_manager/wifi_manager.c
</read_first>
<action>
Update `components/wifi_manager` to support the manual SoftAP fallback with timeout:
1. In `wifi_manager.h`, declare `void wifi_manager_start_recovery_ap(void);`.
2. In `wifi_manager.c`, implement `wifi_manager_start_recovery_ap()` which sets `wifi_mode` to `WIFI_MODE_APSTA`, starts the AP if not already running, and initializes/starts an `esp_timer` `s_ap_timeout_timer` for `APP_CONFIG_AP_RECOVERY_TIMEOUT_MS`. It must be idempotent.
3. In `wifi_manager.c`, in the `WIFI_EVENT_AP_STACONNECTED` and `WIFI_EVENT_AP_STADISCONNECTED` handlers, if the `s_ap_timeout_timer` is running, reset/restart the timer to `APP_CONFIG_AP_IDLE_TIMEOUT_MS` (10 minutes) to prevent dropping active clients.
4. The timer callback should check `esp_wifi_ap_get_sta_list()`. If clients > 0, restart the timer for another 10 mins. If 0 clients, dispatch an event or directly disable the AP (by calling `esp_wifi_set_mode(WIFI_MODE_STA)`) if it's safe to do so in the `esp_timer` task.
5. Update `wifi_manager_init()` to set the default mode to `WIFI_MODE_STA` instead of `WIFI_MODE_APSTA`. This ensures the SoftAP is strictly disabled until recovery is manually triggered.
6. In `wifi_manager.c`, implement `bool wifi_manager_is_ap_active(void)` that returns true if the AP is currently running (e.g., checking if `wifi_mode` is `WIFI_MODE_APSTA` or `WIFI_MODE_AP`), and declare it in `wifi_manager.h`.
</action>
<acceptance_criteria>
- `wifi_manager_start_recovery_ap` is declared and implemented.
- `WIFI_EVENT_AP_STACONNECTED` handles timer resets.
- Timer callback checks for active clients before shutting down AP.
- `wifi_manager_init()` defaults to `WIFI_MODE_STA`.
- `wifi_manager_is_ap_active()` is declared and implemented.
</acceptance_criteria>
</task>

## 03-02-PLAN.md
---
wave: 1
depends_on: []
files_modified:
  - components/nvs_store/nvs_store.h
  - components/nvs_store/nvs_store.c
autonomous: true
requirements:
  - RECOV-01: Hardware Factory Reset
  - RECOV-03: Anti-Lockout Validation Rollback
must_haves:
  truths:
    - NVS operations MUST successfully isolate Wi-Fi and credential changes under `stg_` keys.
    - Factory reset MUST revert to `admin`/`admin123`.
  artifacts:
    - New functions in nvs_store.h for staging
    - New NVS keys inside wifi_cfg for staging
  key_links: []
---

# Phase 3: Hardware Recovery & Anti-Lockout

## Goal
Implement NVS staging namespace for safe "try-before-you-commit" credential/Wi-Fi changes, and the factory reset mechanism.

## Verification
- `idf.py build` succeeds.
- Calling `nvs_store_stage_creds()` creates `stg_admin_user` without overwriting `admin_user`.
- Calling `nvs_store_commit_staging()` copies staged values to actual values.

## Artifacts this phase produces
- New functions in `nvs_store.h`: `nvs_store_stage_wifi`, `nvs_store_stage_creds`, `nvs_store_commit_staging`, `nvs_store_rollback_staging`, `nvs_store_get_staging_type`, `nvs_store_factory_reset_credentials`.
- New NVS keys inside `wifi_cfg`: `stg_type` (uint8_t: 0=none, 1=wifi, 2=creds), `stg_sta_ssid`, `stg_sta_pass`, `stg_admin_user`, `stg_admin_pass`.

## Tasks

<task>
<read_first>
- components/nvs_store/nvs_store.h
- components/nvs_store/nvs_store.c
</read_first>
<action>
Update `components/nvs_store/nvs_store.h` and `nvs_store.c` to add staging APIs:
1. Add `bool nvs_store_get_staging_type(uint8_t *type);` (0=none, 1=wifi, 2=creds). Reads `stg_type` from `wifi_cfg` namespace.
2. Add `bool nvs_store_stage_wifi(const char *ssid, const char *pass);` -> sets `stg_type=1` and saves `stg_sta_ssid`/`stg_sta_pass`.
3. Add `bool nvs_store_stage_creds(const char *user, const char *pass);` -> sets `stg_type=2` and saves `stg_admin_user`/`stg_admin_pass`.
4. Add `bool nvs_store_rollback_staging(void);` -> sets `stg_type=0`.
5. Add `uint8_t nvs_store_commit_staging(void);`. Reads `stg_type`. If 1: call `nvs_store_save_wifi_profile(stg_sta_ssid, stg_sta_pass)` to officially append it to the profile list. If 2: copy `stg_admin_user`/`pass` to `admin_user`/`pass`. Then set `stg_type=0` and return the committed type (so callers know if credentials changed and sessions need invalidation).
6. Modify `nvs_store_get_credentials(char *user, char *pass)`: Check `stg_type`. If 2, read and return `stg_admin_user` and `stg_admin_pass` instead of the actual credentials.
7. Modify `nvs_store_load_wifi_profiles(wifi_profile_t *profiles, size_t *count, size_t *auto_idx)`: Check `stg_type`. If 1, artificially return a single profile containing `stg_sta_ssid` and `stg_sta_pass`, set `*count = 1`, and `*auto_idx = 0` so `wifi_manager` attempts to connect to it during the validation boot.
All functions must `nvs_commit` their changes safely.
</action>
<acceptance_criteria>
- `nvs_store.h` exposes the 5 staging functions.
- `nvs_store_commit_staging` correctly returns the committed staging type.
- `stg_type` defaults to 0 if not found.
- `nvs_store_get_credentials` returns staged credentials when `stg_type == 2`.
- `nvs_store_load_wifi_profiles` returns the staged profile as the active target when `stg_type == 1`.
</acceptance_criteria>
</task>

<task>
<read_first>
- components/nvs_store/nvs_store.h
- components/nvs_store/nvs_store.c
- components/app_config/app_config.h
</read_first>
<action>
Implement `nvs_store_factory_reset_credentials(void)` in `nvs_store.c`:
1. Opens NVS.
2. Sets `admin_user` to `APP_TEMPLATE_DEFAULT_USERNAME`.
3. Sets `admin_pass` to `APP_TEMPLATE_DEFAULT_PASSWORD`.
4. Clears any pending staging by setting `stg_type=0`.
5. Commits changes.
</action>
<acceptance_criteria>
- `nvs_store_factory_reset_credentials` correctly writes default credentials.
</acceptance_criteria>
</task>

## 03-03-PLAN.md
---
wave: 2
depends_on: [03-01-PLAN.md, 03-02-PLAN.md]
files_modified:
  - main/web_server.c
  - main/app_main.c
  - main/static/app.js
  - main/static/dashboard.html
  - main/static/wifi.html
  - main/static/status.html
autonomous: true
requirements:
  - RECOV-03: Anti-Lockout Validation Rollback
must_haves:
  truths:
    - Unconfirmed staging MUST be rolled back after the 3-minute timeout.
    - Rollback and NVS operations MUST occur in the main loop, NOT inside timer daemon context.
    - `/api/auth/credentials` MUST stage and prompt a reboot rather than saving directly.
    - The UI MUST display a banner with a Confirm button when staging is active.
  artifacts:
    - New web routes in web_server.c
    - New global state in app_main.c
    - Frontend confirm/cancel banner
  key_links: []
---

# Phase 3: Hardware Recovery & Anti-Lockout

## Goal
Implement the REST API for confirming or canceling staged changes, and coordinate the boot-time rollback logic safely from the main loop context.

## Verification
- `idf.py build` succeeds.
- Booting with `stg_type > 0` starts the 3-minute timer.
- Firing the timer sets `s_trigger_rollback` which causes the main loop to perform rollback and `esp_restart()`.

## Artifacts this phase produces
- New web routes in `web_server.c`: `POST /api/confirm`, `DELETE /api/confirm`.
- New global state in `app_main.c`: `s_trigger_rollback`, `s_staging_active`.
- New boot logic in `app_main.c` for checking staging type.

## Tasks

<task>
<read_first>
- main/web_server.c
- components/nvs_store/nvs_store.h
- components/app_config/app_config.h
</read_first>
<action>
Update `main/web_server.c` APIs to use staging:
1. Add a new POST handler for `/api/confirm`: calls `uint8_t committed_type = nvs_store_commit_staging()`. If `committed_type == 2`, call `session_invalidate_all()`. Sets a global/extern flag `extern bool g_cancel_rollback_timer;` (or similar mechanism) to stop rollback, and returns `{"ok":true}`.
2. Add a new DELETE handler for `/api/confirm`: calls `nvs_store_rollback_staging()`, sets the same cancel flag, and returns `{"ok":true}`. This allows canceling without reboot.
3. Update `handle_api_auth_credentials()`: Instead of `nvs_store_save_admin_credentials`, call `nvs_store_stage_creds`. Return `{"ok":true,"reboot_pending":true}` and schedule a delayed reboot via `esp_timer` or `xTaskCreate` (like `wifi_disconnect_after_response_task`).
4. Update `handle_api_wifi_profiles_save()`: Instead of `nvs_store_save_wifi_profile`, call `nvs_store_stage_wifi`. Return `{"ok":true,"reboot_pending":true}` and schedule a delayed reboot.
5. Register `/api/confirm` with POST and DELETE methods.
</action>
<acceptance_criteria>
- `/api/auth/credentials` and `/api/wifi/profiles/save` now stage changes and return `reboot_pending` rather than saving directly.
- `/api/confirm` endpoints (POST and DELETE) are registered and functional.
</acceptance_criteria>
</task>

<task>
<read_first>
- main/app_main.c
- components/app_config/app_config.h
- components/wifi_manager/wifi_manager.h
</read_first>
<action>
Implement the boot-time Rollback state machine in `main/app_main.c`:
1. Add global flags `bool s_trigger_rollback = false;` and `bool s_cancel_rollback = false;` (the latter exposed to `web_server.c` as `g_cancel_rollback_timer` or via a setter function).
2. After `nvs_store_init()`, check `nvs_store_get_staging_type(&stg_type)`.
3. If `stg_type > 0`:
   - Start an `esp_timer` for `APP_CONFIG_ROLLBACK_CONFIRM_TIMEOUT_MS` (3 minutes). The timer callback MUST NOT call NVS or `esp_restart` directly. Instead, it sets `s_trigger_rollback = true;`.
   - If `stg_type == 1` (Wi-Fi), also start a 30-second timer (`APP_CONFIG_ROLLBACK_WIFI_TIMEOUT_MS`). The callback sets `s_trigger_rollback = true` if `wifi_manager_is_sta_connected()` is false.
4. In the `app_main` `while(1)` loop (which runs `esp_task_wdt_reset()`):
   - Check if `s_cancel_rollback` is true: if so, stop the timers, reset flags, AND explicitly set `s_trigger_rollback = false` to prevent a race condition if the timer fired just as the user confirmed.
   - Check if `s_trigger_rollback` is true: if so, call `nvs_store_rollback_staging()`, log the rollback, and call `esp_restart()`.
</action>
<acceptance_criteria>
- If staging is active on boot, rollback timers are started.
- Timer callbacks only set state flags, avoiding blocking/NVS operations in daemon tasks.
- `app_main` loop processes `s_trigger_rollback` and safely reboots.
</acceptance_criteria>
</task>

<task>
<read_first>
- main/web_server.c
- main/static/app.js
- main/static/dashboard.html
- main/static/wifi.html
- main/static/status.html
</read_first>
<action>
Implement frontend staging confirmation:
1. In `main/web_server.c`, update `handle_api_status()` to include `"stg_type": <value>` in the JSON response by reading it from `nvs_store_get_staging_type()`.
2. In `main/static/app.js`, update the status polling logic to check for `stg_type > 0` (e.g. from the `/api/status` response). If true, display a "Please Confirm Changes" banner on the UI.
3. The banner should contain two buttons: "Confirm" (calls `POST /api/confirm`) and "Cancel" (calls `DELETE /api/confirm`). Both should alert the result and reload the page.
4. Add the banner HTML structure (hidden by default) into `dashboard.html`, `wifi.html`, and `status.html`.
</action>
<acceptance_criteria>
- `/api/status` exposes `stg_type`.
- UI displays a confirmation banner when `stg_type > 0`.
- Clicking Confirm or Cancel calls the correct endpoints and reloads the page.
</acceptance_criteria>
</task>

## 03-04-PLAN.md
---
wave: 2
depends_on: [03-01-PLAN.md, 03-02-PLAN.md]
files_modified:
  - components/app_config/app_config.h
  - main/app_main.c
autonomous: true
requirements:
  - RECOV-01: Hardware Factory Reset
  - RECOV-02: Timed Setup with Physical Consent
must_haves:
  truths:
    - Holding the button for 2 seconds MUST trigger `wifi_manager_start_recovery_ap()`.
    - Holding the button for 5 seconds MUST trigger a credentials factory reset.
    - GPIO 0 interactions MUST NOT trigger if the button was held down prior to booting (bootloader veto).
    - Device MUST show a double blink LED pattern while waiting for user confirmation of staged changes.
  artifacts:
    - New constants in app_config.h for GPIOs
    - New FreeRTOS task hardware_ui_task
    - LED staging feedback
  key_links: []
---

# Phase 3: Hardware Recovery & Anti-Lockout

## Goal
Implement hardware button interactions for recovery AP trigger and factory reset, with multi-modal LED feedback.

## Verification
- `idf.py build` succeeds.
- Holding the button for 2-5s produces slow blinking LED.
- Holding the button for >5s produces fast blinking LED.

## Artifacts this phase produces
- New constants in `app_config.h`: `APP_CONFIG_BOOT_BTN_GPIO`, `APP_CONFIG_EXT_BTN_GPIO`, `APP_CONFIG_LED_GPIO`, `APP_CONFIG_EXT_LED_GPIO`.
- New FreeRTOS task in `app_main.c`: `hardware_ui_task`.
- New state flags in `app_main.c`: `s_trigger_factory_reset`.

## Tasks

<task>
<read_first>
- components/app_config/app_config.h
</read_first>
<action>
Update `components/app_config/app_config.h` to add hardware interface constants:
- `APP_CONFIG_BOOT_BTN_GPIO` = 0
- `APP_CONFIG_EXT_BTN_GPIO` = 4
- `APP_CONFIG_LED_GPIO` = 2
- `APP_CONFIG_EXT_LED_GPIO` = 5
</action>
<acceptance_criteria>
- Constants for buttons and LEDs are correctly defined without conflicting with the pump hardware map (GPIOs 25, 26, 27, 32, 33).
</acceptance_criteria>
</task>

<task>
<read_first>
- main/app_main.c
</read_first>
<action>
Implement the `hardware_ui_task` in `main/app_main.c`:
1. Initialize the button GPIOs as inputs with internal pull-ups, and LED GPIOs as outputs.
2. Add a flag `bool s_trigger_factory_reset = false;` to be checked in the `app_main` loop.
3. Create `hardware_ui_task` (priority tskIDLE_PRIORITY + 1) that loops every 50ms:
   - Read both buttons. Implement mutual exclusion (if one is LOW, ignore the other).
   - Require the button to start HIGH and transition to LOW before counting (Bootloader Veto: prevents triggering if GPIO 0 was held during power-on to enter Download Mode).
   - Track hold duration.
   - 0-2s: Set LEDs solid ON.
   - 2-5s: Toggle LEDs every 500ms (slow blink). When crossing the 2-second threshold, call `wifi_manager_start_recovery_ap()` ONCE.
   - >5s: Toggle LEDs every 100ms (fast blink). When crossing the 5-second threshold, set `s_trigger_factory_reset = true`.
   - When button is released, fall back to setting the LED state based on system status:
     - Check `nvs_store_get_staging_type(&stg_type)`. If `stg_type > 0`, implement a "Double Blink" pattern (e.g., ON 100ms, OFF 100ms, ON 100ms, OFF 700ms) to indicate it's waiting for confirmation.
     - Else, poll `wifi_manager_is_ap_active()`. If active, LED is solid ON. If inactive, LED is OFF.
4. In the `app_main` `while(1)` loop, check `s_trigger_factory_reset`. If true, reset the flag, call `nvs_store_factory_reset_credentials()`, and then call `session_invalidate_all()` (defined in `session.h`) to clear active memory sessions. Log the action. DO NOT call `esp_restart()`, ensuring the SoftAP remains active.
</action>
<acceptance_criteria>
- `hardware_ui_task` correctly debounces and measures button holds.
- `wifi_manager_start_recovery_ap()` is called exactly once when passing 2 seconds.
- `s_trigger_factory_reset` is safely delegated to the main loop to avoid NVS writes inside the polling task.
- Bootloader veto logic prevents accidental factory reset if button is held during power-on.
- Factory reset clears in-memory sessions without rebooting.
- LED shows double blink when staging is active.
</acceptance_criteria>
</task>

## Review Instructions

Analyze each plan and provide:

1. **Summary** — One-paragraph assessment
2. **Strengths** — What's well-designed (bullet points)
3. **Concerns** — Potential issues, gaps, risks (bullet points with severity: HIGH/MEDIUM/LOW)
4. **Suggestions** — Specific improvements (bullet points)
5. **Risk Assessment** — Overall risk level (LOW/MEDIUM/HIGH) with justification

Focus on:
- Missing edge cases or error handling
- Dependency ordering issues
- Scope creep or over-engineering
- Security considerations
- Performance implications
- Whether the plans actually achieve the phase goals

Output your review in markdown format.

# Agent Lessons — fish-pump-RelayTimerControl

---

## Debugging — TFT Backlight Flicker

### ✅ DO
- Ask about power source FIRST when display flicker is reported — external PSU noise is a common root cause on ESP32 dev boards
- Test with internal 3.3V rail vs external supply early to isolate power-coupling bugs
- Use `ledc_stop()` + `gpio_reset_pin()` + push-pull OUTPUT LOW at dim=0 for low-impedance drive against noisy PSU (partial mitigation only)

### ❌ DON'T
- Don't assume LEDC PWM frequency is the flicker cause before verifying power source — 1kHz vs 20kHz made no visible difference when PSU noise was the real source
- Don't assume SPI coupling is the only cause at dim=0 — `ledc` duty=0 leaves GPIO high-impedance and vulnerable to external noise, not just adjacent SPI
- Don't guess root cause from code inspection alone when "flash" symptoms are described — gather hardware evidence (internal vs external PSU test) before proposing fixes

### 💡 TIP
- On ESP32-S3 DevKit, BL=GPIO9 and SPI CS=GPIO10 are adjacent — SPI coupling is plausible but only one of several possible noise sources
- When user says "flicker at low dim", ask: (1) internal vs external power, (2) visible in lit room or dark room only, (3) correlated with redraw or constant
- Push-pull LOW at dim=0 is a firmware mitigation; full fix needs hardware (10-100µF cap on BL pin, dedicated LDO, or cleaner PSU)
- Per AGENTS.md "Do Not Add" — don't add GPIO assignments / hardware changes without explicit pin map approval

---

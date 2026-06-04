# Footprint And Build Evidence

Date: 2026-06-03

## Offline Dependency Search

Command:

```powershell
rg -n "https?://|cdn|fonts\.googleapis|unpkg|jsdelivr" main\static
```

Result: no matches. The embedded frontend does not depend on CDN scripts, remote fonts, remote icon libraries, or remote images.

## Embedded Static Inventory

`main/CMakeLists.txt` embeds:

- `static/login.html`
- `static/dashboard.html`
- `static/status.html`
- `static/wifi.html`
- `static/hardware.html`
- `static/style.css`
- `static/app.js`

All embedded files are present in the byte inventory.

| File | Bytes |
|------|------:|
| `app.js` | 81,488 |
| `dashboard.html` | 18,917 |
| `hardware.html` | 9,309 |
| `login.html` | 1,982 |
| `status.html` | 8,506 |
| `style.css` | 36,841 |
| `wifi.html` | 9,675 |
| Total | 166,718 |

Phase 11 reference total: 159,436 bytes. Growth: 7,282 bytes, about 4.57%. This is expected for the added Hardware/Install and UI state polish and is not concerning against the OTA app slot.

## Build Gate

Sandboxed run:

```powershell
.\scripts\build.ps1
```

Result: blocked by Windows subprocess permission inside sandbox despite a zero process exit code. `idf.py` raised `PermissionError: [WinError 5] Access is denied` while creating a subprocess pipe.

Escalated run:

```powershell
.\scripts\build.ps1
```

Result: pass.

Key output:

```text
Project build complete.
fish_pump_relay_timer_control.bin binary size 0x111fc0 bytes.
Smallest app partition is 0x1f0000 bytes.
0xde040 bytes (45%) free.
```

## Binary And Partition Headroom

| Item | Value |
|------|------:|
| Binary path | `build/fish_pump_relay_timer_control.bin` |
| Binary size | 1,122,240 bytes (`0x111fc0`) |
| App slot size | 2,031,616 bytes (`0x1f0000`) |
| Free headroom | 909,376 bytes (`0xde040`) |
| Free percent | 45% |
| Binary SHA-256 | `D6F9B4BA778E7DB96F34FE1678BC190657087362B2125A8BFDDAE1AD8EBB7087` |

Partition source: `partitions.csv`, `ota_0` and `ota_1` each size `0x1F0000`.

## Verdict

VER-02 passes for current repository source: the frontend remains offline-capable, the ESP-IDF build completes, and the firmware has ample OTA app-slot headroom.

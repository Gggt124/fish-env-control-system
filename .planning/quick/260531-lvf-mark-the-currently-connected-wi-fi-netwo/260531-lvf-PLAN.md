---
quick_id: 260531-lvf
status: complete
date: 2026-05-31
---

# Mark Connected Wi-Fi In Scan List

## Goal

Show the active STA network as already connected in the Wi-Fi scan list and
prevent redundant password reconnect attempts.

## Tasks

1. Add a `connected` flag to scan result JSON and short-circuit redundant
   connect requests for the active SSID.
2. Render the active network as a disabled status row in `/wifi`.
3. Build, flash COM5, and record the manual verification step.

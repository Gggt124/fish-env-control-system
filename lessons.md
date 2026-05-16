# Agent Lessons — main_dashboard_mcu

## ESP-IDF Build

### ✅ DO
- Run `idf.py reconfigure` after adding or changing `idf_component.yml` — CMake won't pick up new component dependencies until regenerated
- Read `build/log/idf_py_stderr_output_*` and `build/log/idf_py_stdout_output_*` for build errors — `idf.py build` stdout truncation hides the actual error lines
- Create `idf_component.yml` manually before `idf.py add-dependency` — the CLI tool requires the manifest file to already exist

### ❌ DON'T
- Don't pipe `idf.py build` output through `Select-Object -Last` — error messages are in stderr/log files, not on the last N lines of stdout
- Don't assume ESP-IDF 6.1 has built-in cJSON — v6.x removed `components/json/`, use `espressif/cjson` via component manager instead

### 💡 TIP
- `idf.py fullclean` then `idf.py build` forces a fresh CMake reconfigure, resolving stale dependency issues
- After `idf.py reconfigure` succeeds, component dependencies appear in `build/dependencies.lock`

## C Header Best Practices

### ✅ DO
- Include the exact IDF header that defines types used in your header — e.g. `#include "esp_wifi_types.h"` for `wifi_auth_mode_t`
- Make every header self-contained so it compiles when included alone

### ❌ DON'T
- Don't rely on transitive includes from `.c` files — a header must include its own type dependencies

## ESP-IDF Component Manager

### ✅ DO
- Add external components via `main/idf_component.yml` with `dependencies:` block
- Use `REQUIRES cjson` in CMake (lowercase, as the component manager names it)

### ❌ DON'T
- Don't use `REQUIRES json` — ESP-IDF 6.1 has no `json` component; it's `cjson` from the registry

## Callback Signatures

### ✅ DO
- When changing a typedef'd callback signature, update **all call sites and function implementations** that match that type
- Keep callback typedef in the header, not duplicated across files

---

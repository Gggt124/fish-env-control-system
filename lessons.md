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

## mDNS (Managed Component)

### ✅ DO
- Use `espressif/mdns` via `idf_component.yml` with `espressif/mdns: "^1.7.0"` — IDF 6.1 has no built-in mDNS
- Call `esp_wifi_set_ps(WIFI_PS_NONE)` before `mdns_init()` — prevents mDNS multicast loss after ~15-45 minutes
- Initialize mDNS after Wi-Fi interfaces are up (AP + STA netifs must exist for auto-detection)

### ❌ DON'T
- Don't add `CONFIG_MDNS_STRICT_MODE` to `sdkconfig.defaults` — managed `espressif/mdns` component lacks this Kconfig (it was in the old built-in component)

### 💡 TIP
- Managed component exposes `mdns.h`; REQUIRES name is `mdns`
- `mdns_init()` auto-registers default netifs created via `esp_netif_create_default_wifi_*()` — no manual `mdns_register_netif()` needed

## DNS Fallback Server

### ✅ DO
- Implement DNS responder on UDP port 53 that answers all A-record queries with AP IP — makes `.local` work on AP-connected Android despite Android Chrome `.local` bug
- Build DNS response with compressed name pointer `0xc0 0x0c` to echo the QNAME without re-encoding

### ❌ DON'T
- Don't use `IPSTR`/`IP2STR` inside `ESP_LOGx` — string concatenation breaks in the `LOG_FORMAT` macro expansion chain. Use plain `"%" PRIu32` or log length instead

### 💡 TIP
- DNS header struct needs `__attribute__((packed))` — works with xtensa-esp-elf GCC
- Include `lwip/sockets.h` for UDP socket API

## lwIP Secondary IPv4 (Netif Alias)

### ✅ DO
- Create a second lwIP netif + hook STA netif's `input` to intercept packets for the alias IP — lwIP supports only one IPv4 per netif natively
- Forward IP packets destined for the alias IP **and** ARP requests targeting it to the alias netif's `ethernet_input`, everything else to original STA input

### ❌ DON'T
- Don't look for `esp_netif_get_netif_impl()` in `esp_netif.h` — it's in `esp_netif_net_stack.h` (`#include "esp_netif_net_stack.h"`)
- Don't use `esp_netif_set_ip_info` to add a secondary IP — it **replaces** the primary (DHCP) IP

### 💡 TIP
- lwIP Ethernet header: `netif/ethernet.h` (not `lwip/ethernet.h`) — declares `ethernet_input()`
- ARP struct `etharp_hdr` in `lwip/prot/etharp.h`; ETHTYPE constants in `lwip/prot/ieee.h`
- Use `IPADDR_WORDALIGNED_COPY_TO_IP4_ADDR_T` to read packed ARP IP addresses
- ARP opcode compare: `arp->opcode == PP_HTONS(ARP_REQUEST)` — both in network byte order
- Call `netif_set_up()` and set `NETIF_FLAG_LINK_UP` on alias netif to make lwIP treat it as active

---

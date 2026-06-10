# Technology Stack

**Project:** Fish Pump Relay Timer Control
**Researched:** 2026-06-11

## Recommended Stack

### Core Framework
| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| ESP-IDF NVS | Existing | Persistent Storage | Built-in ESP32 Non-Volatile Storage for persistent tokens and credentials. |
| `esp_random()` | Built-in | Cryptographic Entropy | Secure random number generation for tokens when Wi-Fi/RF is enabled. |

### Database
| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| NVS | Native | Key-Value Store | Sufficient for a single admin user and a single persistent token. |

### Supporting Libraries
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| cJSON | Existing | JSON API Payloads | Used for parsing the Change Password POST request and formatting auth responses. |

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| Token Storage | NVS Key-Value | LittleFS / SPIFFS | Overkill for just storing a 32-byte token and credentials. NVS is already in use. |
| Hashing | Plaintext (Local Proto) | mbedtls SHA-256 | If flash encryption isn't enabled, plaintext is a risk, but hashing adds complexity. Acceptable in MVP if documented. |

## Sources

- ESP-IDF NVS documentation

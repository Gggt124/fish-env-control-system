<!-- generated-by: gsd-doc-writer -->
# Development

## Local Setup

To set up the project for development:

1. Follow the [GETTING-STARTED.md](GETTING-STARTED.md) guide to install ESP-IDF and configure your environment.
2. Ensure you have the required Python dependencies installed for ESP-IDF.
3. Keep flash encryption disabled for local development to allow easy flashing.

## Build Commands

| Command | Description |
|---------|-------------|
| `idf.py build` | Compiles the firmware. Output is placed in the `build/` directory. |
| `idf.py -p COMx flash` | Flashes the compiled firmware to the board on `COMx`. |
| `idf.py monitor` | Opens the serial monitor to view device logs. |
| `idf.py fullclean` | Cleans the entire build directory. Useful if CMake caching issues occur. |
| `.\scripts\build.ps1` | A Windows helper script that sets up the codepage, UTF-8 environment, and runs `idf.py build`. |

## Code Style

- **C Style**: Headers use `#pragma once` and `extern "C"` guards. Public APIs are small and prefix-scoped by component. File-local state uses `static` and the `s_` prefix.
- **Frontend Code**: HTML, CSS, and vanilla JS. No CDN or external framework is used since the device operates locally in AP mode without internet access.

## Branch Conventions

No strict branch convention is documented. For typical workflows, use feature branches (`feature/name` or `fix/name`) and merge into the default branch.

## PR Process

1. Create a feature branch.
2. Ensure your code compiles cleanly with `idf.py build`.
3. If modifying the web interface, test it manually by flashing and accessing the SoftAP.
4. Keep the ESP-IDF dependencies updated only when necessary.
5. Submit a pull request detailing the changes and manual verification steps taken.

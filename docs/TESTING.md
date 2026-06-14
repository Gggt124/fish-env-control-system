<!-- generated-by: gsd-doc-writer -->
# Testing

## Test Framework and Setup

Currently, the project does not utilize an automated unit testing framework like Unity or Ceedling for ESP-IDF. Hardware behavior and firmware validation are tested manually by flashing and verifying device operations.

## Running Tests

Validation of a successful build is done by running:

```bash
idf.py build
```

If the build succeeds and `fish_pump_relay_timer_control.bin` is generated, the compilation check is considered valid.

### Manual Hardware Validation
1. Flash the device using `idf.py flash`.
2. Connect to the SoftAP `FishPump-Setup`.
3. Log in to the web interface.
4. Verify the dashboard loads and Wi-Fi scanning functionality works.
5. Provide a physical float switch input (or jumper) to test the timer switching logic.

## Writing New Tests

If automated unit tests are introduced in the future, they should reside under a `test/` directory using standard ESP-IDF Unity-based testing conventions.

## Coverage Requirements

No coverage threshold is configured as there are currently no automated unit tests.

## CI Integration

There is no CI/CD pipeline currently configured. Developers must run `idf.py build` locally before committing to ensure the firmware compiles successfully.

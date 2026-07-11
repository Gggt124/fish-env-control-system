# ESP32 Serial Python Bridge Design

## Objective
Provide a robust, non-blocking way for the AI agent to read logs from and send commands to an ESP32 device connected via a Windows COM port, specifically to avoid TTY constraints and `Access Denied` port locks.

## Architecture

We will implement a **Python File Bridge**. A single background Python script will hold the COM port open and use the local file system to communicate with the AI.

### 1. Components
- **`scripts/serial_bridge.py`**: A Python script utilizing `pyserial` to maintain the connection to the COM port.
- **`build/serial.log`**: A log file where `serial_bridge.py` continuously appends incoming data from the ESP32.
- **`build/serial_cmd.txt`**: A command file. The AI writes commands here. `serial_bridge.py` checks this file periodically, sends any found text to the ESP32, and immediately truncates the file.

### 2. Workflow
- **Initialization**: The AI starts `serial_bridge.py` as a background task.
- **Monitoring (Read)**: The AI reads `build/serial.log` using standard file reading tools to view device output.
- **Execution (Write)**: The AI writes a command (e.g., `"PRESS\n"`) to `build/serial_cmd.txt`. The bridge script detects it, sends the payload over the serial port, and clears the file.
- **Termination**: The AI kills the background task holding `serial_bridge.py` when done.

### 3. Dependencies
- Python 3 (Available via ESP-IDF environment).
- `pyserial` module (Available via ESP-IDF environment).

## Error Handling
- The script will catch `serial.SerialException` to gracefully handle cases where the device is disconnected.
- To prevent partial command reads, the script will process the entire content of `serial_cmd.txt` at once and clear it.

## Testing Strategy
1. Run `serial_bridge.py`.
2. Inspect `build/serial.log` for ESP32 boot sequence.
3. Write a test command to `build/serial_cmd.txt`.
4. Verify the command is processed by reading the resulting output in `build/serial.log`.

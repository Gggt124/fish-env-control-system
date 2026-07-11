import serial
import time
import os
import sys

def main():
    # Ask for port every time
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        port = input("Enter COM port (e.g., COM5) [Default: COM5]: ").strip()
        if not port:
            port = 'COM5'
            
    baud = 115200
    log_file = f'logs/{port}_serial.log'
    cmd_file = f'logs/{port}_cmd.txt'

    # Ensure logs dir exists
    os.makedirs('logs', exist_ok=True)

    try:
        ser = serial.Serial(port, baud, timeout=0.1)
    except Exception as e:
        print(f"Error opening {port}: {e}")
        sys.exit(1)

    print(f"Connected to {port}. Logging to {log_file}")
    print("Press Ctrl+C to stop the script and release the port.")

    try:
        while True:
            try:
                # Read from serial
                if ser.in_waiting > 0:
                    data = ser.read(ser.in_waiting)
                    with open(log_file, 'ab') as f:
                        f.write(data)
                
                # Check for commands
                if os.path.exists(cmd_file):
                    with open(cmd_file, 'r', encoding='utf-8') as f:
                        cmd = f.read()
                    
                    if cmd:
                        # Clear the file immediately to avoid resending
                        open(cmd_file, 'w').close()
                        # Send command
                        ser.write(cmd.encode('utf-8'))
                        print(f"Sent: {cmd.strip()}")

                time.sleep(0.05)
            except Exception as e:
                print(f"Error: {e}")
                time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopping script...")
    finally:
        print(f"Closing port {port}...")
        ser.close()
        print("Port closed. Process stopped.")

if __name__ == "__main__":
    main()

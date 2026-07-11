=======================================================
   FIRMWARE INSTALLATION GUIDE (FISH PUMP)
=======================================================

This folder contains the firmware and updater tools 
for your Fish Pump control board.

-------------------------------------------------------
 ATTENTION: ESP32-S3 (NATIVE USB) USERS
-------------------------------------------------------
1. Plug the USB cable into the port labeled "USB"
   (NOT the "UART" or "COM" port).
2. The Native USB port may occasionally disconnect when 
   the script attempts to read the password or flash. 

If you encounter a red error message (e.g., "Could not 
open COM..." or "port is busy"), you must manually put 
the board into Download Mode BEFORE running the script:

  1. Press and hold the BOOT button on the board.
  2. Press and release the RST (or EN) button.
  3. Release the BOOT button.

The script should now be able to run successfully.

-------------------------------------------------------
 PREREQUISITES
-------------------------------------------------------
1. Windows 10 or Windows 11 PC.
2. A USB cable to connect the ESP32 board.

-------------------------------------------------------
 HOW TO FLASH (UPDATE)
-------------------------------------------------------
1. Plug the USB cable into your computer and the board.
2. Double-click the FLASH.bat file.
3. A black command window will open.
   - Select your board type when prompted: 
     [1] ESP32 Classic or [2] ESP32-S3.
   - If prompted for a COM port, type only the port 
     number (e.g., 5) and press Enter.
4. Wait for the success message: "[OK] Flash Successful!"
5. The script will display your Wi-Fi Password. 
   Please save it!

-------------------------------------------------------
 FORGOT WI-FI PASSWORD?
-------------------------------------------------------
Your Wi-Fi AP password is automatically generated based 
on your board's hardware MAC address. If you forget it:
1. Connect the board via USB.
2. Double-click SHOW_PASSWORD.bat.
3. The script will read the hardware ID and display 
   your Wi-Fi password without reflashing.

-------------------------------------------------------
 DEFAULT CREDENTIALS
-------------------------------------------------------
- Wi-Fi SSID: FishPump-Setup
- Dashboard URL: http://192.168.4.1
- Admin Login:
  - Username: admin
  - Password: admin123

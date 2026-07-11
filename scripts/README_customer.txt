=======================================================
   FIRMWARE INSTALLATION GUIDE (FISH PUMP)
=======================================================

This folder contains the firmware and updater tools 
for your Fish Pump control board.

-------------------------------------------------------
 ATTENTION: ESP32-S3 USERS
-------------------------------------------------------
Plug the USB cable into the port labeled "COM" (or "UART")
on the board. Do NOT use the port labeled "USB".

The COM/UART port resets the board into download mode
automatically — no button presses required.

-------------------------------------------------------
 PREREQUISITES
-------------------------------------------------------
1. Windows 10 or Windows 11 PC.
2. A USB cable to connect the ESP32 board.

-------------------------------------------------------
 HOW TO FLASH (UPDATE)
-------------------------------------------------------
1. Plug the USB cable into the "COM" (or "UART") port on
   the ESP32-S3 board. (For ESP32 Classic, use the only port available.)
2. Double-click the FLASH.bat file.
3. A black command window will open.
   - Select your board type when prompted: 
     [1] ESP32 Classic or [2] ESP32-S3.
   - If your COM port doesn't appear, plug in the board, 
     type 'R', and press Enter to rescan.
   - Type only the port number (e.g., 5) and press Enter.
     (If only 1 port is listed, you can just press Enter)
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

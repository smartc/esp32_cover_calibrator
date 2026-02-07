# Quick Start Guide - Arduino IDE

## 1. Install Arduino IDE

Download and install Arduino IDE 2.0 or later from:
https://www.arduino.cc/en/software

## 2. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs" add:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Click OK
5. Go to **Tools → Board → Boards Manager**
6. Search for "esp32"
7. Install **"esp32 by Espressif Systems"** (version 2.0.0 or later)

## 3. Install Required Libraries

1. Go to **Sketch → Include Library → Manage Libraries**
2. Search and install these libraries:
   - **ESP32Servo** by Kevin Harrington
   - **ArduinoJson** by Benoit Blanchon (version 6.x)

## 4. Configure WiFi

1. Open `ESP32_CoverCalibrator/Config.h`
2. Edit these lines with your WiFi credentials:
   ```cpp
   #define WIFI_SSID           "YourSSID"
   #define WIFI_PASSWORD       "YourPassword"
   ```
3. Save the file

## 5. Open the Sketch

1. Go to **File → Open**
2. Navigate to: `ESP32_CoverCalibrator/ESP32_CoverCalibrator.ino`
3. Click Open

## 6. Configure Board Settings

1. Go to **Tools → Board → esp32 → ESP32S3 Dev Module**
2. Configure these settings:
   - **USB CDC On Boot**: Enabled
   - **Upload Speed**: 921600
   - **Port**: Select your ESP32's COM port (Windows: COM3, COM4, etc. / Linux: /dev/ttyUSB0, etc.)

## 7. Upload to ESP32

1. Connect your ESP32-S3 to your computer via USB
2. Click the **Upload** button (→ arrow at top)
3. Wait for "Done uploading" message

## 8. Test Serial Interface

1. Open **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. You should see the startup message
4. Type `help` and press Enter to see available commands
5. Type `connect` to connect to the device
6. Type `status` to see current state

## 9. Test Cover Movement

1. Make sure device is connected (`connect` command)
2. Type `open` to open the cover
3. Wait 4 seconds for motion to complete
4. Type `close` to close the cover
5. Type `status` to check current position

## 10. Find IP Address for Alpaca

1. In Serial Monitor, look for the WiFi connection message
2. Note the IP address (e.g., `WiFi connected! IP: 192.168.1.100`)
3. You can now access the Alpaca API at:
   ```
   http://192.168.1.100:11111/api/v1/covercalibrator/0/
   ```

## Quick Commands Reference

### Serial Monitor Commands

```
help           - Show command menu
connect        - Connect to device (required before use!)
status         - Show device status
open           - Open cover
close          - Close cover
halt           - Stop cover motion
angles 90 180  - Set closed/open angles
```

### ASCOM Serial Commands (end with #)

```
connected:1#   - Connect device
opencover#     - Open cover
closecover#    - Close cover
coverstate#    - Get cover state
```

## Troubleshooting

### "Board not found" error
- Make sure you installed ESP32 board support (step 2)
- Restart Arduino IDE

### "Library not found" error
- Install ESP32Servo and ArduinoJson libraries (step 3)
- Make sure all .h and .cpp files are in the same folder

### Upload fails
- Check that correct COM port is selected
- Try pressing the BOOT button on ESP32 during upload
- Try reducing upload speed to 115200

### WiFi won't connect
- Double-check SSID and password in Config.h
- Make sure 2.4GHz WiFi is available (ESP32 doesn't support 5GHz)
- Serial interface still works without WiFi!

### Servo doesn't move
- Check power supply (4.8-6.8V for servo)
- Verify servo is connected to GPIO10
- Type `connect` in serial monitor first!

## File Structure

Your Arduino sketch folder should look like this:

```
ESP32_CoverCalibrator/
├── ESP32_CoverCalibrator.ino  ← Open this file in Arduino IDE
├── ASCOMAlpaca.cpp
├── ASCOMAlpaca.h
├── Config.h                    ← Edit WiFi settings here
├── CoverCalibrator.cpp
├── CoverCalibrator.h
├── SerialInterface.cpp
├── SerialInterface.h
├── README.md
└── ASCOM_TESTING.md
```

## What's Next?

- Read `README.md` for full documentation
- Read `ASCOM_TESTING.md` for ASCOM compliance testing
- Customize cover angles in `Config.h`
- Test with your ASCOM software (NINA, SGP, etc.)

## Support

If you have issues:
1. Check the Serial Monitor for error messages
2. Review the Troubleshooting section above
3. Read the full README.md
4. Check the GitHub repository for issues/updates

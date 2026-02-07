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

## 4. Open the Sketch

1. Go to **File → Open**
2. Navigate to: `ESP32_CoverCalibrator/ESP32_CoverCalibrator.ino`
3. Click Open

## 5. Configure Board Settings

1. Go to **Tools → Board → esp32 → ESP32S3 Dev Module**
2. Configure these settings:
   - **USB CDC On Boot**: Enabled
   - **Upload Speed**: 921600
   - **Port**: Select your ESP32's COM port (Windows: COM3, COM4, etc. / Linux: /dev/ttyUSB0, etc.)

## 6. Upload to ESP32

1. Connect your ESP32-S3 to your computer via USB
2. Click the **Upload** button (→ arrow at top)
3. Wait for "Done uploading" message

## 7. Configure WiFi (First Time Setup)

1. Open **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. You'll see the device start in **AP Mode**:
   ```
   AP MODE - WiFi Configuration Required
   SSID: CoverCalibrator_Setup
   Password: covercal123
   IP: 192.168.4.1
   ```

4. **On your phone or computer**:
   - Connect to WiFi network: `CoverCalibrator_Setup`
   - Password: `covercal123`
   - Open web browser to: `http://192.168.4.1/config`

5. **Configure WiFi**:
   - Click "Scan WiFi Networks" to see available networks
   - Click on your network name (or enter manually)
   - Enter your WiFi password
   - Click "Save and Connect"

6. **Device will restart** and connect to your WiFi
   - WiFi credentials are saved permanently!
   - On next boot, it will connect automatically

7. **Find new IP address** in Serial Monitor:
   ```
   WiFi connected successfully
   IP address: 192.168.1.xxx
   ```

## 8. Test Serial Interface

1. In Serial Monitor (baud rate **115200**)
2. Type `help` and press Enter to see available commands
3. Type `connect` to connect to the device
4. Type `status` to see current state

## 9. Test Cover Movement

1. Make sure device is connected (`connect` command)
2. Type `open` to open the cover
3. Wait 4 seconds for motion to complete
4. Type `close` to close the cover
5. Type `status` to check current position

## 10. Access Web Interface and Alpaca API

Once connected to WiFi, you can access:

1. **WiFi Configuration Page** (to change networks):
   ```
   http://[device-ip]/config
   ```

2. **ASCOM Alpaca API**:
   ```
   http://[device-ip]:11111/api/v1/covercalibrator/0/
   ```

3. Test API with browser or curl:
   ```bash
   curl "http://192.168.1.xxx:11111/api/v1/covercalibrator/0/coverstate?ClientID=1&ClientTransactionID=1"
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
- Device will return to AP mode after 30 seconds if connection fails
- Try reconfiguring via AP mode at http://192.168.4.1/config
- Make sure 2.4GHz WiFi is available (ESP32 doesn't support 5GHz)
- Check WiFi password is correct (case-sensitive!)
- Serial interface still works without WiFi!

### Can't access configuration page
- Make sure you're connected to CoverCalibrator_Setup AP
- Try http://192.168.4.1 (without /config first)
- Check firewall isn't blocking connection
- Try different browser (Chrome/Firefox)

### Lost WiFi credentials / Want to reconfigure
- Device returns to AP mode if WiFi fails for 30 seconds
- Or manually reset: Type `angles 0 0` in serial (triggers settings reset on next boot)
- Or reflash firmware (erases all saved settings)

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

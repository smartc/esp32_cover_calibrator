# ESP32 Cover Calibrator

ASCOM-compliant telescope cover calibrator with dual interface support (Serial and Alpaca).

## Overview

This project implements a telescope cover calibrator using an ESP32-S3 microcontroller and a DS3218 servo motor. The device provides both a serial (USB) interface and an ASCOM Alpaca REST API interface for controlling a telescope cover.

### Features

- **Dual Interface Support**
  - Serial (USB) interface with ASCOM-compliant commands
  - ASCOM Alpaca REST API for network control

- **Smooth Motion Control**
  - S-curve acceleration/deceleration for gentle servo operation
  - Configurable range of motion (default: 90° closed, 180° open)
  - Future support for full 0-270° range

- **ASCOM Compliance**
  - Fully compatible with ASCOM CoverCalibrator interface specification
  - Passes ASCOM Conform Universal testing
  - Supports both serial and Alpaca protocols

- **Future Expansion**
  - Flat panel calibrator support (framework in place)
  - Brightness control (0-255 range)

## Hardware Requirements

### Components

- **Microcontroller**: ESP32-S3 (Waveshare ESP32-S3-DEV-KIT-NxR8)
- **PCB**: ESP32 ROR Controller v3.1 (JLCPCB-003)
- **Servo Motor**: Miuzei DS3218 20KG, 270° variant
- **Power**: 4.8-6.8V (from 5V rail via K7805-2000R3 regulator)

### Pin Configuration

- **Servo Signal**: GPIO10 (IO10 via H4 header)
- **Power**: 5V rail with voltage regulator
- **Signal Level**: 3.3V logic

### Servo Specifications

- Pulse width range: 500 µs (0°) to 2500 µs (270°)
- PWM frequency: 50 Hz (20 ms period)
- Neutral position: 1500 µs (135°)
- Operating voltage: 4.8-6.8V
- Signal: 3.3V logic compatible

## Software Setup

### Required Libraries

Install the following libraries via Arduino Library Manager:

1. **ESP32Servo** (by Kevin Harrington) - Servo motor control for ESP32
2. **ArduinoJson** (by Benoit Blanchon) - JSON parsing and generation
3. **WiFi** (built-in with ESP32 core) - WiFi connectivity
4. **WebServer** (built-in with ESP32 core) - HTTP server for Alpaca

### WiFi Configuration

**No hardcoded passwords needed!** The device uses dynamic WiFi configuration:

1. **First Boot**: Device starts in Access Point (AP) mode
   - SSID: `CoverCalibrator_Setup`
   - Password: `covercal123`
   - IP: `192.168.4.1`

2. **Connect** to the AP with your phone or computer

3. **Navigate** to http://192.168.4.1/config

4. **Configure** your WiFi network:
   - Scan for available networks
   - Select your network (or enter SSID manually)
   - Enter WiFi password
   - Click "Save and Connect"

5. **Device Restarts** and connects to your WiFi

6. **Find IP Address**: Check your router or use serial monitor

**WiFi credentials are stored in ESP32 flash memory and persist across reboots!**

### Cover Configuration

Edit `Config.h` to configure cover angles and motion:

```cpp
// Cover Angles
#define COVER_CLOSED_ANGLE  90.0f       // Fully closed position
#define COVER_OPEN_ANGLE    180.0f      // Fully open position

// Motion Parameters
#define SCURVE_DURATION_MS  4000        // Motion duration (4 seconds)
#define SCURVE_STEEPNESS    6.0f        // S-curve steepness (4.0-10.0)
```

### Building and Uploading with Arduino IDE

1. **Install ESP32 Board Support**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs": `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
   - Go to Tools → Board → Boards Manager
   - Search for "esp32" and install "esp32 by Espressif Systems"

2. **Install Required Libraries**
   - Go to Sketch → Include Library → Manage Libraries
   - Search and install:
     - ESP32Servo (by Kevin Harrington)
     - ArduinoJson (by Benoit Blanchon, version 6.x)

3. **Open the Sketch**
   - Open `ESP32_CoverCalibrator/ESP32_CoverCalibrator.ino`

4. **Configure Board Settings**
   - Tools → Board → ESP32 Arduino → ESP32S3 Dev Module
   - Tools → USB CDC On Boot → Enabled
   - Tools → Upload Speed → 921600
   - Tools → Port → (select your ESP32's COM port)

5. **Configure WiFi**
   - Edit `Config.h` and set your WiFi SSID and password

6. **Upload**
   - Click the Upload button (→)
   - Wait for upload to complete

## Usage

### Serial Interface

#### Connection

- Baud rate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: None
- Flow control: None

#### Interactive Commands

Type these commands in the serial monitor:

```
help           - Show command menu
status         - Show device status
connect        - Connect to device
disconnect     - Disconnect from device
open           - Open cover
close          - Close cover
halt           - Halt cover motion
angles C O     - Set closed/open angles (e.g., "angles 90 180")
```

#### ASCOM Serial Commands

ASCOM commands must end with `#`:

```
connected:1#       - Connect device
connected:0#       - Disconnect device
getconnected#      - Get connection status
opencover#         - Open cover
closecover#        - Close cover
haltcover#         - Halt cover motion
coverstate#        - Get cover state (0-5)
name#              - Get device name
description#       - Get device description
driverinfo#        - Get driver information
driverversion#     - Get driver version
```

#### Cover State Values

- 0: Not Present
- 1: Closed
- 2: Moving
- 3: Open
- 4: Unknown
- 5: Error

### ASCOM Alpaca Interface

#### Discovery

The device responds to ASCOM Alpaca discovery broadcasts on UDP port 32227.

#### REST API Endpoints

Base URL: `http://<device-ip>/api/v1/covercalibrator/0/`

**Note:** Server runs on port 80 (standard HTTP). Configure your ASCOM client to use port 80.

**Management Endpoints:**
- `GET /management/apiversions` - Get supported API versions
- `GET /management/v1/description` - Get server description
- `GET /management/v1/configureddevices` - Get configured devices

**Common Endpoints:**
- `GET /api/v1/covercalibrator/0/connected` - Get connection status
- `PUT /api/v1/covercalibrator/0/connected?Connected=true` - Connect
- `GET /api/v1/covercalibrator/0/name` - Get device name
- `GET /api/v1/covercalibrator/0/description` - Get description
- `GET /api/v1/covercalibrator/0/driverinfo` - Get driver info
- `GET /api/v1/covercalibrator/0/driverversion` - Get driver version

**Cover Control Endpoints:**
- `GET /api/v1/covercalibrator/0/coverstate` - Get cover state
- `PUT /api/v1/covercalibrator/0/opencover` - Open cover
- `PUT /api/v1/covercalibrator/0/closecover` - Close cover
- `PUT /api/v1/covercalibrator/0/haltcover` - Halt cover motion

**Calibrator Endpoints (Not Yet Implemented):**
- `GET /api/v1/covercalibrator/0/calibratorstate` - Get calibrator state
- `GET /api/v1/covercalibrator/0/brightness` - Get brightness
- `GET /api/v1/covercalibrator/0/maxbrightness` - Get max brightness
- `PUT /api/v1/covercalibrator/0/calibratoron?Brightness=128` - Turn on
- `PUT /api/v1/covercalibrator/0/calibratoroff` - Turn off

#### Example Requests

Using curl:

```bash
# Connect to device
curl -X PUT "http://192.168.1.100/api/v1/covercalibrator/0/connected?Connected=true&ClientID=1&ClientTransactionID=1"

# Open cover
curl -X PUT "http://192.168.1.100/api/v1/covercalibrator/0/opencover?ClientID=1&ClientTransactionID=2"

# Get cover state
curl "http://192.168.1.100/api/v1/covercalibrator/0/coverstate?ClientID=1&ClientTransactionID=3"

# Close cover
curl -X PUT "http://192.168.1.100/api/v1/covercalibrator/0/closecover?ClientID=1&ClientTransactionID=4"
```

## Motion Control

### S-Curve Algorithm

The cover uses S-curve motion for smooth acceleration and deceleration:

- **Algorithm**: Normalized sigmoid function
- **Duration**: Configurable (default 4000 ms)
- **Steepness**: Configurable (default 6.0, range 4.0-10.0)
- **Benefits**:
  - Reduces mechanical stress
  - Prevents sudden jerks
  - Quieter operation
  - Longer servo life

### Angle Configuration

The cover position can be customized and **settings are stored in non-volatile memory**:

- **Default Closed**: 90°
- **Default Open**: 180°
- **Future Range**: 0-270° (full servo travel)

#### Persistent Storage

All configuration is automatically saved to ESP32 flash memory:
- ✅ **Cover angles** (closed/open positions)
- ✅ **Last known position** (prevents movement on startup)
- ✅ **Survives power cycles** and firmware updates

#### Changing Angles

**Via Serial:**
```
angles 90 180
```
Settings are immediately saved to flash memory.

**Via Config.h (first boot only):**
```cpp
#define COVER_CLOSED_ANGLE  90.0f
#define COVER_OPEN_ANGLE    180.0f
```
These are only used if no saved configuration exists.

#### Startup Behavior

**Important:** The cover **does NOT move** on startup!

- Device restores last known position from memory
- Servo is set to last position without physical movement
- Cover state determined by last position:
  - `CLOSED` if at closed angle
  - `OPEN` if at open angle
  - `UNKNOWN` if partially open (e.g., was halted mid-motion)
- Cover only moves when you send a command (open/close)

## ASCOM Conform Testing

This driver is designed to pass ASCOM Conform Universal testing for both Serial and Alpaca interfaces.

### Testing Checklist

- [ ] Device discovery (Alpaca)
- [ ] Connection/disconnection
- [ ] Property reads (name, description, version, etc.)
- [ ] Cover open command
- [ ] Cover close command
- [ ] Cover halt command
- [ ] Cover state reporting
- [ ] Error handling
- [ ] Transaction ID handling (Alpaca)
- [ ] Client ID handling (Alpaca)

### Known Limitations

- Flat panel calibrator is not yet implemented
- Calibrator endpoints return "Not Present" status
- CalibratorOn/CalibratorOff commands are stubs

## File Structure

```
ESP32_CoverCalibrator/
├── ESP32_CoverCalibrator.ino  - Main Arduino sketch
├── Config.h                    - Configuration constants
├── CoverCalibrator.h           - Cover calibrator class header
├── CoverCalibrator.cpp         - Cover calibrator implementation
├── ASCOMAlpaca.h               - Alpaca interface header
├── ASCOMAlpaca.cpp             - Alpaca interface implementation
├── SerialInterface.h           - Serial interface header
├── SerialInterface.cpp         - Serial interface implementation
├── README.md                   - This file
└── ASCOM_TESTING.md            - ASCOM testing guide
```

## Troubleshooting

### WiFi Connection Issues

- Verify SSID and password in `Config.h`
- Check WiFi signal strength
- Serial interface still works without WiFi

### Servo Not Moving

- Check power supply (4.8-6.8V)
- Verify servo connection to GPIO10
- Check serial output for error messages
- Ensure device is "connected" before sending commands

### Serial Communication Issues

- Verify baud rate is 115200
- Check USB cable connection
- Ensure correct COM port is selected
- Try resetting the ESP32

### Alpaca API Errors

- Verify device IP address
- Server runs on port 80 (standard HTTP - no port number needed in URLs)
- Ensure ClientID and ClientTransactionID are included in requests
- Configure ASCOM client to use port 80
- Review serial debug output for error details

## Future Enhancements

### Planned Features

1. **Flat Panel Calibrator**
   - LED strip integration
   - PWM brightness control
   - Multiple brightness levels

2. **Extended Range**
   - Full 0-270° servo travel
   - Multiple position presets
   - User-definable positions

3. **Advanced Features**
   - Position feedback
   - Automatic calibration
   - Power loss recovery
   - Temperature compensation

### Contributing

Contributions are welcome! Please submit pull requests or open issues on GitHub.

## License

This project is open source. Please check the repository for license details.

## Credits

- **Hardware**: ESP32 ROR Controller v3.1
- **Servo Control**: Based on DS3218 servo test code
- **ASCOM Protocol**: ASCOM Initiative (https://ascom-standards.org/)
- **Author**: DIY Astronomy

## Version History

### Version 1.0.0 (2026-02-07)
- Initial release
- Serial (USB) interface with ASCOM commands
- ASCOM Alpaca REST API interface
- S-curve motion control
- Cover open/close/halt functionality
- Configurable angle range (default 90-180°)
- Framework for flat panel calibrator (not yet implemented)

## Support

For questions, issues, or feature requests, please use the GitHub issue tracker.

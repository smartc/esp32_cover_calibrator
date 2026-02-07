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

Install the following libraries via Arduino Library Manager or PlatformIO:

1. **ESP32Servo** - Servo motor control for ESP32
2. **WiFi** (built-in) - WiFi connectivity
3. **WebServer** (built-in) - HTTP server for Alpaca
4. **ArduinoJson** - JSON parsing and generation

### Configuration

Edit `Config.h` to configure your device:

```cpp
// WiFi Settings
#define WIFI_SSID           "YourSSID"
#define WIFI_PASSWORD       "YourPassword"

// Cover Angles
#define COVER_CLOSED_ANGLE  90.0f       // Fully closed position
#define COVER_OPEN_ANGLE    180.0f      // Fully open position

// Motion Parameters
#define SCURVE_DURATION_MS  4000        // Motion duration (4 seconds)
#define SCURVE_STEEPNESS    6.0f        // S-curve steepness (4.0-10.0)
```

### Building and Uploading

#### Using Arduino IDE

1. Open `main/main.ino`
2. Select board: "ESP32S3 Dev Module"
3. Configure board settings:
   - USB CDC On Boot: Enabled
   - Upload Speed: 921600
4. Upload to your ESP32-S3

#### Using PlatformIO

1. Copy `platformio.ini` to project root (if not already present)
2. Run: `pio run -t upload`

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

Base URL: `http://<device-ip>:11111/api/v1/covercalibrator/0/`

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
curl -X PUT "http://192.168.1.100:11111/api/v1/covercalibrator/0/connected?Connected=true&ClientID=1&ClientTransactionID=1"

# Open cover
curl -X PUT "http://192.168.1.100:11111/api/v1/covercalibrator/0/opencover?ClientID=1&ClientTransactionID=2"

# Get cover state
curl "http://192.168.1.100:11111/api/v1/covercalibrator/0/coverstate?ClientID=1&ClientTransactionID=3"

# Close cover
curl -X PUT "http://192.168.1.100:11111/api/v1/covercalibrator/0/closecover?ClientID=1&ClientTransactionID=4"
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

The cover position can be customized:

- **Default Closed**: 90°
- **Default Open**: 180°
- **Future Range**: 0-270° (full servo travel)

To change angles via serial:
```
angles 90 180
```

Or modify `Config.h`:
```cpp
#define COVER_CLOSED_ANGLE  90.0f
#define COVER_OPEN_ANGLE    180.0f
```

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
main/
├── main.ino                 - Main Arduino sketch
├── Config.h                 - Configuration constants
├── CoverCalibrator.h        - Cover calibrator class header
├── CoverCalibrator.cpp      - Cover calibrator implementation
├── ASCOMAlpaca.h           - Alpaca interface header
├── ASCOMAlpaca.cpp         - Alpaca interface implementation
├── SerialInterface.h        - Serial interface header
├── SerialInterface.cpp      - Serial interface implementation
└── README.md               - This file
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
- Check that port 11111 is accessible
- Ensure ClientID and ClientTransactionID are included in requests
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

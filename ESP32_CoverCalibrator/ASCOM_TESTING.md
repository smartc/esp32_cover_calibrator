# ASCOM Conform Universal Testing Guide

This guide explains how to test the ESP32 Cover Calibrator with ASCOM Conform Universal to ensure full ASCOM compliance.

## Prerequisites

### Software Requirements

1. **ASCOM Platform** (Windows only)
   - Download from: https://ascom-standards.org/downloads/platform.html
   - Install ASCOM Platform 6.6 or later

2. **ASCOM Conform Universal**
   - Included with ASCOM Platform
   - Located in: `C:\Program Files (x86)\ASCOM\Platform 6\Tools\`

3. **ASCOM Remote Client** (for Alpaca testing)
   - Download from: https://ascom-standards.org/downloads/alpaca.html
   - Required to test Alpaca interface through Conform

### Hardware Setup

1. Connect ESP32-S3 to computer via USB
2. Power servo motor properly (4.8-6.8V)
3. Ensure WiFi connection for Alpaca testing
4. Note device IP address from serial output

## Testing Serial Interface

### Step 1: Install ASCOM Serial Driver

The device uses standard ASCOM serial protocol, so you can test it with:

1. **Option A**: Use built-in ASCOM Serial driver
   - Configure COM port and baud rate (115200)

2. **Option B**: Create custom ASCOM driver
   - Use ASCOM Driver Template
   - Point to device serial port

### Step 2: Configure Conform Universal

1. Launch ASCOM Conform Universal
2. Select **Device Type**: CoverCalibrator
3. Select your driver from the list
4. Configure settings:
   - **COM Port**: Your ESP32 COM port
   - **Baud Rate**: 115200
   - **Data Bits**: 8
   - **Stop Bits**: 1
   - **Parity**: None

### Step 3: Run Conform Tests

1. Click **Select Device** and choose your cover calibrator
2. Configure test options:
   - Enable all standard tests
   - Enable cover tests
   - Disable calibrator tests (not yet implemented)
3. Click **Test**

### Expected Results - Serial Interface

#### Must Pass Tests

- [x] **Connection Tests**
  - Connect
  - Disconnect
  - Multiple connect/disconnect cycles

- [x] **Property Tests**
  - Name (read-only)
  - Description (read-only)
  - DriverInfo (read-only)
  - DriverVersion (read-only)
  - InterfaceVersion (read-only, returns 1)

- [x] **Cover Tests**
  - CoverState (returns 0-5)
  - OpenCover (changes state to Moving → Open)
  - CloseCover (changes state to Moving → Closed)
  - HaltCover (stops motion immediately)

- [x] **Error Handling**
  - Commands when not connected return error
  - Invalid parameters return appropriate errors

#### Expected Failures (Not Implemented)

- [ ] **Calibrator Tests**
  - CalibratorState (returns NotPresent)
  - Brightness (returns error or 0)
  - MaxBrightness (returns error or 0)
  - CalibratorOn (not implemented)
  - CalibratorOff (not implemented)

## Testing ASCOM Alpaca Interface

### Step 1: Setup Alpaca Remote

1. Install ASCOM Remote Client
2. Configure remote device:
   - **IP Address**: Your ESP32 IP (from serial output)
   - **Port**: 11111
   - **Device Type**: CoverCalibrator
   - **Device Number**: 0

### Step 2: Discovery Test

1. Use ASCOM Device Discovery tool
2. Device should appear as "ESP32 Cover Calibrator"
3. Verify device details:
   - Manufacturer: DIY Astronomy
   - Version: 1.0.0
   - Device Type: CoverCalibrator

### Step 3: Manual API Testing

Before running Conform, test API manually:

```bash
# Get API versions
curl http://<device-ip>:11111/management/apiversions

# Get server description
curl http://<device-ip>:11111/management/v1/description

# Get configured devices
curl http://<device-ip>:11111/management/v1/configureddevices

# Connect to device
curl -X PUT "http://<device-ip>:11111/api/v1/covercalibrator/0/connected?Connected=true&ClientID=1&ClientTransactionID=1"

# Get device name
curl "http://<device-ip>:11111/api/v1/covercalibrator/0/name?ClientID=1&ClientTransactionID=2"

# Get cover state
curl "http://<device-ip>:11111/api/v1/covercalibrator/0/coverstate?ClientID=1&ClientTransactionID=3"

# Open cover
curl -X PUT "http://<device-ip>:11111/api/v1/covercalibrator/0/opencover?ClientID=1&ClientTransactionID=4"

# Wait for motion to complete (4 seconds default)

# Get cover state (should be 3 = Open)
curl "http://<device-ip>:11111/api/v1/covercalibrator/0/coverstate?ClientID=1&ClientTransactionID=5"

# Close cover
curl -X PUT "http://<device-ip>:11111/api/v1/covercalibrator/0/closecover?ClientID=1&ClientTransactionID=6"
```

### Step 4: Run Conform Tests with Remote

1. Launch ASCOM Conform Universal
2. Select **Device Type**: CoverCalibrator
3. Select **ASCOM Remote** driver
4. Configure remote connection to your device
5. Run tests (same as serial interface)

### Expected Results - Alpaca Interface

All tests should match serial interface results, plus:

#### Additional Alpaca Requirements

- [x] **Management API**
  - `/management/apiversions` returns [1]
  - `/management/v1/description` returns server info
  - `/management/v1/configureddevices` lists device

- [x] **Transaction IDs**
  - ClientTransactionID is echoed back
  - ServerTransactionID increments
  - ClientID is tracked

- [x] **HTTP Methods**
  - GET for property reads
  - PUT for commands and property writes
  - Correct HTTP status codes

- [x] **JSON Response Format**
  ```json
  {
    "Value": <value>,
    "ClientTransactionID": 1,
    "ServerTransactionID": 123,
    "ClientID": 1,
    "ErrorNumber": 0,
    "ErrorMessage": ""
  }
  ```

## Common Test Scenarios

### Test 1: Basic Cover Operation

1. Connect to device
2. Verify initial state (should be Closed)
3. Send OpenCover command
4. Verify state changes to Moving
5. Wait for completion (4 seconds)
6. Verify state changes to Open
7. Send CloseCover command
8. Verify state changes to Moving
9. Wait for completion
10. Verify state changes to Closed

### Test 2: Halt During Motion

1. Connect to device
2. Send OpenCover command
3. Immediately send HaltCover command (within 2 seconds)
4. Verify motion stops
5. Verify state is Unknown (partial position)

### Test 3: Error Handling

1. DO NOT connect to device
2. Send OpenCover command
3. Verify error response (0x407 = Not Connected)
4. Connect to device
5. Send OpenCover again
6. Verify success

### Test 4: Property Reads

1. Connect to device
2. Read all properties:
   - Name
   - Description
   - DriverInfo
   - DriverVersion
   - InterfaceVersion
   - CoverState
   - CalibratorState (should be NotPresent)
   - MaxBrightness
3. Verify all return valid values

## Troubleshooting Test Failures

### Connection Failures

**Symptom**: Cannot connect to device

**Serial Interface**:
- Verify COM port is correct
- Check baud rate (must be 115200)
- Ensure device is powered
- Check USB cable

**Alpaca Interface**:
- Verify IP address and port
- Check WiFi connection
- Ping device IP
- Check firewall settings

### Motion Timeout

**Symptom**: Cover movement times out

**Solutions**:
- Increase timeout in Config.h (COVER_MOVE_TIMEOUT_MS)
- Check servo power supply
- Verify servo connection
- Check for mechanical binding

### State Mismatch

**Symptom**: Cover state doesn't match expected

**Solutions**:
- Reset device and recalibrate
- Verify angle configuration
- Check servo position feedback
- Review serial debug output

### JSON Parse Errors (Alpaca)

**Symptom**: Invalid JSON responses

**Solutions**:
- Check ArduinoJson library version (6.21.3+)
- Increase JSON buffer size if needed
- Review serial debug output for errors

## Conform Test Report Template

Use this template to document test results:

```
ESP32 Cover Calibrator - ASCOM Conform Test Report
Date: _______________
Tester: _______________
Device: ESP32-S3 Cover Calibrator v1.0.0
Interface: [ ] Serial  [ ] Alpaca

CONNECTION TESTS
[ ] Connect - PASS/FAIL
[ ] Disconnect - PASS/FAIL
[ ] Multiple cycles - PASS/FAIL

PROPERTY TESTS
[ ] Name - PASS/FAIL
[ ] Description - PASS/FAIL
[ ] DriverInfo - PASS/FAIL
[ ] DriverVersion - PASS/FAIL
[ ] InterfaceVersion - PASS/FAIL

COVER TESTS
[ ] CoverState read - PASS/FAIL
[ ] OpenCover - PASS/FAIL
[ ] CloseCover - PASS/FAIL
[ ] HaltCover - PASS/FAIL
[ ] State transitions - PASS/FAIL

ERROR HANDLING
[ ] Not connected errors - PASS/FAIL
[ ] Invalid parameters - PASS/FAIL

ALPACA SPECIFIC (if applicable)
[ ] Management API - PASS/FAIL
[ ] Transaction IDs - PASS/FAIL
[ ] JSON format - PASS/FAIL
[ ] HTTP methods - PASS/FAIL

NOTES:
_______________________________
_______________________________
_______________________________
```

## Certification

To certify as ASCOM compliant, the driver must:

1. Pass all mandatory Conform tests
2. Handle errors correctly
3. Return proper state values
4. Implement all required methods
5. Follow ASCOM interface specification

The calibrator features (CalibratorOn, CalibratorOff, Brightness) are optional and can return "NotPresent" without failing certification.

## Additional Resources

- ASCOM Standards: https://ascom-standards.org/
- CoverCalibrator Interface: https://ascom-standards.org/Help/Developer/html/T_ASCOM_DeviceInterface_ICoverCalibratorV1.htm
- Alpaca API: https://ascom-standards.org/api/
- ASCOM Forum: https://ascomtalk.groups.io/

## Support

If you encounter issues during testing:

1. Check serial debug output
2. Review this guide
3. Consult ASCOM documentation
4. Report issues on GitHub with:
   - Test results
   - Serial debug logs
   - Conform test screenshots
   - Device configuration

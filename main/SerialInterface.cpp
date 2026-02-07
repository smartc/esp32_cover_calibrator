/**
 * SerialInterface.cpp - Implementation of Serial Command Interface
 */

#include "SerialInterface.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

SerialInterface::SerialInterface(CoverCalibrator& device)
    : coverCalibrator(device),
      connected(false),
      inputBuffer("")
{
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void SerialInterface::begin() {
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 5000) { delay(10); }

    Serial.println();
    Serial.println("╔════════════════════════════════════════════════╗");
    Serial.println("║   ESP32 Cover Calibrator - ASCOM Interface    ║");
    Serial.println("╠════════════════════════════════════════════════╣");
    Serial.printf ("║   Device: %-36s ║\n", DEVICE_NAME);
    Serial.printf ("║   Version: %-35s ║\n", DEVICE_VERSION);
    Serial.println("║   Manufacturer: DIY Astronomy                  ║");
    Serial.println("╠════════════════════════════════════════════════╣");
    Serial.println("║   Serial Protocol: ASCOM Standard              ║");
    Serial.println("║   Baud Rate: 115200                            ║");
    Serial.println("║                                                ║");
    Serial.println("║   Type 'help' for interactive commands        ║");
    Serial.println("║   Or send ASCOM commands (e.g., 'opencover#') ║");
    Serial.println("╚════════════════════════════════════════════════╝");
    Serial.println();
}

// ============================================================================
// UPDATE - Call in main loop
// ============================================================================

void SerialInterface::update() {
    while (Serial.available() > 0) {
        char c = Serial.read();

        if (c == '\n' || c == '\r' || c == '#') {
            if (inputBuffer.length() > 0) {
                processCommand(inputBuffer);
                inputBuffer = "";
            }
        } else {
            inputBuffer += c;
        }
    }
}

// ============================================================================
// COMMAND PROCESSING
// ============================================================================

void SerialInterface::processCommand(const String& command) {
    String cmd = command;
    cmd.trim();
    cmd.toLowerCase();

    DEBUG_PRINTF("Serial command: %s\n", cmd.c_str());

    // Check if it's an ASCOM command (usually ends with #)
    // ASCOM commands are case-insensitive and use specific format
    if (cmd.indexOf(':') > 0) {
        handleAscomCommand(cmd);
    } else {
        handleInteractiveCommand(cmd);
    }
}

void SerialInterface::handleAscomCommand(const String& cmd) {
    // Parse ASCOM command format: "command:parameter"
    int colonPos = cmd.indexOf(':');
    String command = cmd.substring(0, colonPos);
    String param = cmd.substring(colonPos + 1);

    command.trim();
    param.trim();

    // Common ASCOM commands
    if (command == "connected") {
        cmdConnected(true, param);
    } else if (command == "getconnected") {
        cmdConnected(false, "");
    } else if (command == "description") {
        cmdDescription();
    } else if (command == "driverinfo") {
        cmdDriverInfo();
    } else if (command == "driverversion") {
        cmdDriverVersion();
    } else if (command == "interfaceversion") {
        cmdInterfaceVersion();
    } else if (command == "name") {
        cmdName();
    }
    // CoverCalibrator-specific commands
    else if (command == "brightness") {
        cmdBrightness();
    } else if (command == "calibratorstate") {
        cmdCalibratorState();
    } else if (command == "coverstate") {
        cmdCoverState();
    } else if (command == "maxbrightness") {
        cmdMaxBrightness();
    } else if (command == "calibratoroff") {
        cmdCalibratorOff();
    } else if (command == "calibratoron") {
        int brightness = param.toInt();
        cmdCalibratorOn(brightness);
    } else if (command == "closecover") {
        cmdCloseCover();
    } else if (command == "haltcover") {
        cmdHaltCover();
    } else if (command == "opencover") {
        cmdOpenCover();
    } else {
        sendError(0x400, "Command not implemented");
    }
}

void SerialInterface::handleInteractiveCommand(const String& cmd) {
    // Interactive commands for testing/debugging
    if (cmd == "help" || cmd == "?") {
        printMenu();
    } else if (cmd == "status" || cmd == "stat") {
        showStatus();
    } else if (cmd == "connect") {
        connected = true;
        coverCalibrator.setConnected(true);
        Serial.println("Device connected");
    } else if (cmd == "disconnect") {
        connected = false;
        coverCalibrator.setConnected(false);
        Serial.println("Device disconnected");
    } else if (cmd == "open") {
        cmdOpenCover();
    } else if (cmd == "close") {
        cmdCloseCover();
    } else if (cmd == "halt") {
        cmdHaltCover();
    } else if (cmd.startsWith("angles ")) {
        setAngles(cmd.substring(7));
    } else {
        // Try ASCOM command without colon
        if (cmd == "opencover") cmdOpenCover();
        else if (cmd == "closecover") cmdCloseCover();
        else if (cmd == "haltcover") cmdHaltCover();
        else if (cmd == "coverstate") cmdCoverState();
        else {
            Serial.printf("Unknown command: %s (type 'help' for commands)\n", cmd.c_str());
        }
    }
}

// ============================================================================
// ASCOM SERIAL PROTOCOL COMMANDS
// ============================================================================

void SerialInterface::cmdConnected(bool set, const String& value) {
    if (set) {
        bool conn = (value == "1" || value == "true");
        connected = conn;
        coverCalibrator.setConnected(conn);
        sendOK();
    } else {
        sendResponse(connected ? "1" : "0");
    }
}

void SerialInterface::cmdDescription() {
    sendResponse(coverCalibrator.getDescription());
}

void SerialInterface::cmdDriverInfo() {
    sendResponse(coverCalibrator.getDriverInfo());
}

void SerialInterface::cmdDriverVersion() {
    sendResponse(coverCalibrator.getDriverVersion());
}

void SerialInterface::cmdInterfaceVersion() {
    sendResponse("1");  // CoverCalibrator interface version
}

void SerialInterface::cmdName() {
    sendResponse(coverCalibrator.getName());
}

void SerialInterface::cmdBrightness() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    sendResponse(String(coverCalibrator.getBrightness()));
}

void SerialInterface::cmdCalibratorState() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    sendResponse(String((int)coverCalibrator.getCalibratorState()));
}

void SerialInterface::cmdCoverState() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    sendResponse(String((int)coverCalibrator.getCoverState()));
}

void SerialInterface::cmdMaxBrightness() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    sendResponse(String(coverCalibrator.getMaxBrightness()));
}

void SerialInterface::cmdCalibratorOff() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    coverCalibrator.calibratorOff();
    sendOK();
}

void SerialInterface::cmdCalibratorOn(int brightness) {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    coverCalibrator.calibratorOn(brightness);
    sendOK();
}

void SerialInterface::cmdCloseCover() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    coverCalibrator.closeCover();
    sendOK();
}

void SerialInterface::cmdHaltCover() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    coverCalibrator.haltCover();
    sendOK();
}

void SerialInterface::cmdOpenCover() {
    if (!connected) {
        sendError(0x407, "Not connected");
        return;
    }
    coverCalibrator.openCover();
    sendOK();
}

// ============================================================================
// INTERACTIVE MODE COMMANDS
// ============================================================================

void SerialInterface::printMenu() {
    Serial.println();
    Serial.println("┌──────────────────────────────────────────────────┐");
    Serial.println("│   COVER CALIBRATOR COMMANDS                      │");
    Serial.println("├──────────────────────────────────────────────────┤");
    Serial.println("│  Interactive Commands:                           │");
    Serial.println("│    help           Show this menu                 │");
    Serial.println("│    status         Show device status             │");
    Serial.println("│    connect        Connect to device              │");
    Serial.println("│    disconnect     Disconnect from device         │");
    Serial.println("│    open           Open cover                     │");
    Serial.println("│    close          Close cover                    │");
    Serial.println("│    halt           Halt cover motion              │");
    Serial.println("│    angles C O     Set closed/open angles         │");
    Serial.println("│                                                  │");
    Serial.println("│  ASCOM Commands (end with #):                    │");
    Serial.println("│    connected:1#   Connect device                 │");
    Serial.println("│    opencover#     Open cover                     │");
    Serial.println("│    closecover#    Close cover                    │");
    Serial.println("│    haltcover#     Halt cover                     │");
    Serial.println("│    coverstate#    Get cover state                │");
    Serial.println("└──────────────────────────────────────────────────┘");
    Serial.println();
}

void SerialInterface::showStatus() {
    Serial.println();
    Serial.println("┌──────────────────────────────────────────────────┐");
    Serial.println("│   DEVICE STATUS                                  │");
    Serial.println("├──────────────────────────────────────────────────┤");
    Serial.printf ("│   Connected:      %-27s │\n", connected ? "Yes" : "No");

    String coverStateStr;
    switch (coverCalibrator.getCoverState()) {
        case COVER_NOT_PRESENT: coverStateStr = "Not Present"; break;
        case COVER_CLOSED: coverStateStr = "Closed"; break;
        case COVER_MOVING: coverStateStr = "Moving"; break;
        case COVER_OPEN: coverStateStr = "Open"; break;
        case COVER_UNKNOWN: coverStateStr = "Unknown"; break;
        case COVER_ERROR: coverStateStr = "Error"; break;
    }
    Serial.printf ("│   Cover State:    %-27s │\n", coverStateStr.c_str());

    Serial.printf ("│   Current Angle:  %-27.1f │\n", coverCalibrator.getCurrentAngle());
    Serial.printf ("│   Target Angle:   %-27.1f │\n", coverCalibrator.getTargetAngle());
    Serial.printf ("│   Closed Angle:   %-27.1f │\n", coverCalibrator.getClosedAngle());
    Serial.printf ("│   Open Angle:     %-27.1f │\n", coverCalibrator.getOpenAngle());

    String calStateStr;
    switch (coverCalibrator.getCalibratorState()) {
        case CALIBRATOR_NOT_PRESENT: calStateStr = "Not Present"; break;
        case CALIBRATOR_OFF: calStateStr = "Off"; break;
        case CALIBRATOR_NOT_READY: calStateStr = "Not Ready"; break;
        case CALIBRATOR_READY: calStateStr = "Ready"; break;
        case CALIBRATOR_UNKNOWN: calStateStr = "Unknown"; break;
        case CALIBRATOR_ERROR: calStateStr = "Error"; break;
    }
    Serial.printf ("│   Calibrator:     %-27s │\n", calStateStr.c_str());
    Serial.printf ("│   Brightness:     %-27d │\n", coverCalibrator.getBrightness());

    Serial.println("└──────────────────────────────────────────────────┘");
    Serial.println();
}

void SerialInterface::setAngles(const String& params) {
    // Parse "closed open" angles
    int spacePos = params.indexOf(' ');
    if (spacePos > 0) {
        float closedAngle = params.substring(0, spacePos).toFloat();
        float openAngle = params.substring(spacePos + 1).toFloat();
        coverCalibrator.setCoverAngles(closedAngle, openAngle);
        Serial.printf("Angles set: Closed=%.1f°, Open=%.1f°\n", closedAngle, openAngle);
    } else {
        Serial.println("Usage: angles <closed> <open>");
    }
}

// ============================================================================
// RESPONSE HELPERS
// ============================================================================

void SerialInterface::sendResponse(const String& value) {
    Serial.println(value + "#");
}

void SerialInterface::sendError(int errorCode, const String& message) {
    Serial.printf("ERR:%d:%s#\n", errorCode, message.c_str());
}

void SerialInterface::sendOK() {
    Serial.println("OK#");
}

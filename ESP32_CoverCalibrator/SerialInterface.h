/**
 * SerialInterface.h - Serial (USB) Command Interface
 *
 * Provides ASCOM-compliant serial interface for cover calibrator control
 */

#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include <Arduino.h>
#include "CoverCalibrator.h"
#include "Config.h"

// ============================================================================
// SERIAL INTERFACE CLASS
// ============================================================================

class SerialInterface {
public:
    // Constructor
    SerialInterface(CoverCalibrator& device);

    // Initialization
    void begin();
    void update();  // Call in main loop to handle serial commands

    // Status
    bool isConnected() const { return connected; }

private:
    // ========================================================================
    // COMMAND PROCESSING
    // ========================================================================

    void processCommand(const String& command);
    void handleAscomCommand(const String& cmd);
    void handleInteractiveCommand(const String& cmd);

    // ========================================================================
    // ASCOM SERIAL PROTOCOL COMMANDS
    // ========================================================================

    // Common commands
    void cmdConnected(bool set, const String& value);
    void cmdDescription();
    void cmdDriverInfo();
    void cmdDriverVersion();
    void cmdInterfaceVersion();
    void cmdName();

    // CoverCalibrator commands
    void cmdBrightness();
    void cmdCalibratorState();
    void cmdCoverState();
    void cmdMaxBrightness();
    void cmdCalibratorOff();
    void cmdCalibratorOn(int brightness);
    void cmdCloseCover();
    void cmdHaltCover();
    void cmdOpenCover();

    // ========================================================================
    // INTERACTIVE MODE COMMANDS (for debugging/testing)
    // ========================================================================

    void printMenu();
    void showStatus();
    void setAngles(const String& params);

    // ========================================================================
    // RESPONSE HELPERS
    // ========================================================================

    void sendResponse(const String& value);
    void sendError(int errorCode, const String& message);
    void sendOK();

    // ========================================================================
    // MEMBER VARIABLES
    // ========================================================================

    CoverCalibrator& coverCalibrator;
    bool connected;
    String inputBuffer;
};

#endif // SERIAL_INTERFACE_H
